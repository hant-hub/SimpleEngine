{ pkgs ? (import <nixpkgs> {}) }: with pkgs;

mkShell {
  packages = with pkgs; [
        xorg.libX11.dev
        vulkan-tools
        vulkan-headers
        vulkan-loader
        vulkan-validation-layers
        vulkan-tools
        shaderc
        renderdoc
  ];
}
