import utils from "./utils.js";

const settings = {
  run: async (Module) => {
    utils.assert(
      Module.Image.getSupportedFormats().includes(".png"),
      "supported formats",
    );

    const img1 = new Module.Image(10, 10, 4, Module.ImageChannelType.BYTE);

    const pngBuffer = img1.saveBuffer(Module.ImageSaveFormat.PNG);

    const img2 = new Module.Image.createFromBuffer(pngBuffer);

    utils.assert(
        img1.equals(img2), "image from buffer stream comparison"
    );
  },
};

utils.runBasicTest(settings);
