RSL_SHADERS = KMLuna.sl KMPlanetclouds.sl KMPuffyclouds.sl KMVenus.sl LGStarfield.sl LGVeinedMarble.sl LGbrushedmetal.sl Lava2.sl SHW_velvet.sl TLOrange.sl brick.sl bw.sl checker.sl checker_aa.sl checker_simple.sl cyclone.sl cyclone2.sl dented.sl diffuse.sl for.sl glass.sl glass2.sl granite.sl hair.sl metal.sl mirror.sl normal_s.sl paintedplastic.sl parquet_plank.sl parquet_plank_aa.sl phong.sl plastic.sl screen.sl screen_aa.sl shinymetal.sl siggraph10_tv.sl skin.sl specular.sl starball.sl texture.sl texture_mix.sl turbulence.sl whitted.sl wood.sl

all: ${RSL_SHADERS}

%.sl:
	bash compile_rsl_shader.sh ${DEVENV_DIR}/rtfact/shaders/src/rsl/$@ &> /dev/null
