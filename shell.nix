{ pkgs ? import <nixpkgs> {}, stdenv ? pkgs.stdenvNoCC }:
let
	bochs_gdb = stdenv.mkDerivation rec {
		pname = "bochs-gdb";
		version = "2.6.7";

		nativeBuildInputs = with pkgs.buildPackages; [
			gcc7
			xorg.libX11
			xorg.libXrandr
		];

		src = fetchTarball {
			url = "https://sourceforge.net/projects/bochs/files/bochs/${version}/bochs-${version}.tar.gz/download";
		};

		configureFlags = [
			"--enable-gdb-stub"
			"--with-x"
		];
	};
	bochs_nogdb = bochs_gdb.overrideAttrs (oldAttr: {
		pname = "bochs-nogdb";

		nativeBuildInputs = with pkgs.buildPackages; [ 
			pkg-config
			gtk2
			readline
		] ++ oldAttr.nativeBuildInputs;
	
		configureFlags = [
			"--enable-debugger"
			"--enable-disasm"
			"--enable-x86-debugger"
			"--enable-readline"
			"--with-x"
		];

		postInstall = "mv $out/bin/bochs $out/bin/bochs_nogdb";
	});
in
	pkgs.mkShell {
		nativeBuildInputs = with pkgs.buildPackages; [
			dev86
			gcc7
			gdb
			bochs_gdb
			bochs_nogdb
			bear
			clang-tools
		];
		hardeningDisable = ["all"];
	}
