{
  description = "pkgmgr development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = import nixpkgs {
      inherit system;
    };
  in
  {
    devShells.${system}.default = pkgs.mkShell {
      packages = with pkgs; [
        clang
        llvm
	lld
        curl
        pkg-config
        zsh
      ];

      shellHook = ''
       	export PROMPT="develop %~ > " 
        echo "pkgmgr dev shell"
        echo "clang: $(clang --version | head -n1)"
      '';
    };
  };
}
