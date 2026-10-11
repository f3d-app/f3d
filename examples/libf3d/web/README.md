# Web examples

In order to run the examples, F3D must be built locally, which is done automatically but requires Docker installed on your computer.

## Specific example

It's possible to run one specific example by navigating to its folder (e.g. `examples/libf3d/web/simple-ui`) and install dependencies by running:

```bash
npm install
```

Once dependencies are installed (can take a while because F3D is compiled), the example can be launched by running:

```bash
npm run dev
```

## All examples

It's possible to install dependencies once by running the following command from `examples/libf3d/web`:

```bash
npm install
```

Then it's possible to select the example to launch. In order to run `simple-ui`, the following command must be run:

```bash
npm run dev --workspace simple-ui
```
