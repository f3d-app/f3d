import utils from "./utils.js";

const settings = {
  run: (Module) => {
    const color = new Module.Color(0.1, 0.2, 0.3);
    const identical = new Module.Color(0.1, 0.2, 0.3);
    utils.assert(
      color.compare(identical, 0.0),
      "Identical colors should compare equal",
    );
    identical.delete();

    for (let component = 0; component < 3; component++) {
      const values = [0.1, 0.2, 0.3];
      values[component] += 0.0000005;
      const near = new Module.Color(...values);
      utils.assert(
        color.compare(near, 1e-6),
        "Nearby colors should compare equal",
      );
      utils.assert(
        near.compare(color, 1e-6),
        "Color comparison should be symmetric",
      );
      utils.assert(
        !color.compare(near, 0.0),
        "Zero tolerance should compare exactly",
      );

      values[component] += 0.000002;
      const different = new Module.Color(...values);
      utils.assert(
        !color.compare(different, 1e-6),
        "Different colors should not compare equal",
      );
      near.delete();
      different.delete();
    }

    const zero = new Module.Color(0.0, 0.0, 0.0);
    const boundary = new Module.Color(0.0, 0.0, 1e-6);
    utils.assert(
      zero.compare(boundary, 1e-6),
      "Colors at the tolerance should compare equal",
    );
    color.delete();
    zero.delete();
    boundary.delete();
  },
};

utils.runBasicTest(settings);
