import utils from "./utils.js";

const settings = {
  run: async (Module) => {
    utils.assert(
      Module.Image.getSupportedFormats().includes(".png"),
      "supported formats",
    );

    const img = new Module.Image(10, 10, 4, Module.ImageChannelType.BYTE);

    utils.assert(img.channelTypeSize === 1, "channel type size");
    utils.assert(img.channelCount === 4, "channel count");
    utils.assert(
      img.channelType === Module.ImageChannelType.BYTE,
      "channel type",
    );

    // create a red/blue checkerboard pattern
    let buffer = new Uint8Array(img.width * img.height * img.channelCount);
    for (let i = 0; i < img.width * img.height; i++) {
      let x = Math.floor(i % img.width);
      let y = Math.floor(i / img.width);
      let checker = (x + y) % 2 === 0;

      buffer[i * 4 + 0] = checker ? 255 : 0;
      buffer[i * 4 + 1] = 0;
      buffer[i * 4 + 2] = checker ? 0 : 255;
      buffer[i * 4 + 3] = 255;
    }

    img.setContent(buffer);

    const content = img.getContent();
    utils.assert(
      content.length === img.width * img.height * img.channelCount,
      "image content length",
    );
    utils.assert(
      content[0] === 255 &&
        content[1] === 0 &&
        content[2] === 0 &&
        content[3] === 255 &&
        content[4] === 0 &&
        content[5] === 0 &&
        content[6] === 255 &&
        content[7] === 255,
      "image content of first two pixels",
    );

    const pngBuffer = img.saveBuffer(Module.ImageSaveFormat.PNG);
    utils.assert(pngBuffer.length > 0, "png buffer length");
    utils.assert(
      pngBuffer[0] === 137 &&
        pngBuffer[1] === 80 &&
        pngBuffer[2] === 78 &&
        pngBuffer[3] === 71,
      "png buffer header",
    );

    utils.assert(img.toTerminalText().length === 224, "toTerminalText length");

    console.log("\n" + img.toTerminalText() + "\n");

    await utils.copyLocalFileToWasmFS(
      Module,
      "/src/testing/data/10x10_checker.png",
      "checker.png",
    );

    const baselineImg = new Module.Image("checker.png");
    utils.assert(img.equals(baselineImg), "image equals");

    const normalizedPixel = img.getNormalizedPixel(0, 0);
    utils.assert(normalizedPixel.length === 4, "normalized pixel length");
    utils.assert(
      utils.numArrayEquals(normalizedPixel, [1, 0, 0, 1]),
      "normalized pixel",
    );

    img.setMetadata("Author", "John Doe");
    utils.assert(
      img.allMetadata().includes("Author"),
      "allMetadata includes Author",
    );
    utils.assert(
      img.getMetadata("Author") === "John Doe",
      "getMetadata after setMetadata",
    );
  },
};

utils.runBasicTest(settings);
