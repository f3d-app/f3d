#!/usr/bin/env python3
"""
Compare UV coordinates between DCM and OBJ files to find the correct transformation.
"""

import re
import subprocess
import sys

DCM_FILE = "/Users/romainnosenzo/CLionProjects/Open3SDCM/TestData/real-world/scan_019.dcm"
OBJ_FILE = "/Users/romainnosenzo/CLionProjects/Open3SDCM/Output/2026-05-05-18-51-09.491192/scan_019.obj"
MTL_FILE = "/Users/romainnosenzo/CLionProjects/Open3SDCM/Output/2026-05-05-18-51-09.491192/scan_019.mtl"


def parse_obj_uvs(obj_path):
    """Parse UV coordinates from OBJ file."""
    uvs = []
    with open(obj_path, 'r') as f:
        for line in f:
            line = line.strip()
            if line.startswith('vt '):
                parts = line.split()
                if len(parts) >= 3:
                    u = float(parts[1])
                    v = float(parts[2])
                    uvs.append((u, v))
    return uvs


def parse_dcm_uvs(dcm_path):
    """Use f3d to extract UV coordinates from DCM file."""
    # We'll create a simple C++ program that loads the DCM and prints UVs
    cpp_code = '''
#include "plugins/open3sdcm/module/vtkF3DOpen3SDCMImporter.h"
#include <vtkActor.h>
#include <vtkFloatArray.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 2) return 1;
    
    vtkNew<vtkF3DOpen3SDCMImporter> importer;
    importer->SetFileName(argv[1]);
    
    if (!importer->ImportBegin()) {
        std::cerr << "Failed to import" << std::endl;
        return 1;
    }
    
    vtkNew<vtkRenderWindow> renderWindow;
    vtkNew<vtkRenderer> renderer;
    renderWindow->AddRenderer(renderer);
    importer->SetRenderWindow(renderWindow);
    importer->Update();
    
    vtkActorCollection* actors = importer->GetImportedActors();
    actors->InitTraversal();
    vtkActor* actor = actors->GetNextActor();
    
    if (!actor) {
        std::cerr << "No actors" << std::endl;
        return 1;
    }
    
    vtkPolyData* polyData = vtkPolyData::SafeDownCast(actor->GetMapper()->GetInputDataObject(0));
    if (!polyData) {
        std::cerr << "No polydata" << std::endl;
        return 1;
    }
    
    vtkFloatArray* tcoords = vtkFloatArray::SafeDownCast(polyData->GetPointData()->GetTCoords());
    if (!tcoords) {
        std::cout << "NO_TEXTURE_COORDINATES" << std::endl;
        return 0;
    }
    
    std::cout << "DCM_UV_COORDS" << std::endl;
    for (vtkIdType i = 0; i < tcoords->GetNumberOfTuples(); i++) {
        float* uv = tcoords->GetTuple2(i);
        std::cout << uv[0] << " " << uv[1] << std::endl;
    }
    
    return 0;
}
'''
    
    # Write temporary C++ file
    import os
    import tempfile
    with tempfile.NamedTemporaryFile(mode='w', suffix='.cxx', delete=False) as f:
        f.write(cpp_code)
        cpp_file = f.name
    
    try:
        # Compile and run
        exe_file = '/tmp/extract_dcm_uvs'
        compile_cmd = [
            '/usr/bin/c++', '-std=c++20',
            '-I/Users/romainnosenzo/CLionProjects/f3d-dcm/plugins/open3sdcm/module',
            '-I/Users/romainnosenzo/CLionProjects/f3d-dcm/build-open3sdcm/plugins/open3sdcm/module',
            '-I/Users/romainnosenzo/CLionProjects/f3d-dcm/build-open3sdcm/vtkext/public/module',
            '-I/Users/romainnosenzo/CLionProjects/f3d-dcm/build-open3sdcm/vtkext/private/module',
            '-I/opt/homebrew/include/vtk-9.6',
            '-I/opt/homebrew/include',
            '-I/Users/romainnosenzo/CLionProjects/Open3SDCM/Lib/src',
            cpp_file, '-o', exe_file,
            '-L/Users/romainnosenzo/CLionProjects/f3d-dcm/build-open3sdcm/lib',
            '-lvtkextOpen3SDCM', '-lOpen3SDCMLib', '-lPocoXML', '-lPocoZip', '-lcrypto', '-lz',
            '-lvtkCommonCore-9.6', '-lvtkIOGeometry-9.6', '-lvtkRenderingCore-9.6'
        ]
        
        result = subprocess.run(compile_cmd, capture_output=True, text=True, timeout=60)
        if result.returncode != 0:
            print("Compilation failed:")
            print(result.stderr)
            return None
        
        # Run the executable
        result = subprocess.run([exe_file, dcm_path], capture_output=True, text=True, timeout=30)
        if result.returncode != 0:
            print("Execution failed:")
            print(result.stderr)
            return None
        
        uvs = []
        for line in result.stdout.strip().split('\n'):
            if line == "DCM_UV_COORDS":
                continue
            if line == "NO_TEXTURE_COORDINATES":
                return None
            parts = line.split()
            if len(parts) == 2:
                uvs.append((float(parts[0]), float(parts[1])))
        return uvs
    finally:
        if os.path.exists(cpp_file):
            os.unlink(cpp_file)
        if os.path.exists(exe_file):
            os.unlink(exe_file)


def find_transformation(dcm_uvs, obj_uvs, max_samples=10):
    """Find the transformation between DCM and OBJ UV coordinates."""
    if not dcm_uvs or not obj_uvs:
        return None
    
    # Try all possible transformations
    transformations = [
        ("identity", lambda u, v: (u, v)),
        ("swap", lambda u, v: (v, u)),
        ("flip_v", lambda u, v: (u, 1-v)),
        ("flip_u", lambda u, v: (1-u, v)),
        ("swap_flip_v", lambda u, v: (v, 1-u)),
        ("flip_v_swap", lambda u, v: (1-v, u)),
        ("flip_both", lambda u, v: (1-u, 1-v)),
        ("flip_u_swap", lambda u, v: (1-u, v)),
    ]
    
    best_transform = None
    best_error = float('inf')
    
    # Sample first few points for comparison
    num_compare = min(len(dcm_uvs), len(obj_uvs), max_samples)
    
    for name, transform in transformations:
        total_error = 0
        for i in range(num_compare):
            u_dcm, v_dcm = dcm_uvs[i]
            u_obj, v_obj = obj_uvs[i]
            u_t, v_t = transform(u_dcm, v_dcm)
            error = abs(u_t - u_obj) + abs(v_t - v_obj)
            total_error += error
        
        avg_error = total_error / num_compare
        if avg_error < best_error:
            best_error = avg_error
            best_transform = name
        
        print(f"  {name:20s}: avg_error = {avg_error:.6f}")
    
    return best_transform, best_error


def main():
    print("=" * 60)
    print("Comparing UV coordinates between DCM and OBJ files")
    print("=" * 60)
    
    print(f"\nDCM file: {DCM_FILE}")
    print(f"OBJ file: {OBJ_FILE}")
    
    # Parse OBJ UVs
    print("\nParsing OBJ file...")
    obj_uvs = parse_obj_uvs(OBJ_FILE)
    print(f"Found {len(obj_uvs)} UV coordinates in OBJ")
    
    # Parse DCM UVs
    print("\nParsing DCM file...")
    dcm_uvs = parse_dcm_uvs(DCM_FILE)
    if dcm_uvs is None:
        print("Could not parse DCM UV coordinates")
        return
    print(f"Found {len(dcm_uvs)} UV coordinates in DCM")
    
    # Compare
    print(f"\nComparing first {min(len(dcm_uvs), len(obj_uvs), 5)} UV coordinates:")
    print(f"{'Index':<8} {'DCM_U':<12} {'DCM_V':<12} {'OBJ_U':<12} {'OBJ_V':<12}")
    print("-" * 56)
    for i in range(min(len(dcm_uvs), len(obj_uvs), 5)):
        du, dv = dcm_uvs[i]
        ou, ov = obj_uvs[i]
        print(f"{i:<8} {du:<12.6f} {dv:<12.6f} {ou:<12.6f} {ov:<12.6f}")
    
    print("\nTrying different transformations:")
    best_transform, best_error = find_transformation(dcm_uvs, obj_uvs)
    
    print(f"\n{'='*60}")
    print(f"BEST TRANSFORMATION: {best_transform} (avg_error = {best_error:.6f})")
    print(f"{'='*60}")
    
    # Print the transformation as C++ code
    transform_code = {
        "identity": "{avgU, avgV}",
        "swap": "{avgV, avgU}",
        "flip_v": "{avgU, 1.0f - avgV}",
        "flip_u": "{1.0f - avgU, avgV}",
        "swap_flip_v": "{avgV, 1.0f - avgU}",
        "flip_v_swap": "{1.0f - avgV, avgU}",
        "flip_both": "{1.0f - avgU, 1.0f - avgV}",
        "flip_u_swap": "{1.0f - avgU, avgV}",
    }
    
    if best_transform:
        code = transform_code.get(best_transform, "{avgU, avgV}")
        print(f"\nC++ code to use:")
        print(f"  tcoords->SetTypedTuple(v, std::array<float, 2>{code}.data());")
    
    # Also show what the current code produces
    print(f"\nCurrent code uses: {{avgV, avgU}}")


if __name__ == "__main__":
    main()
