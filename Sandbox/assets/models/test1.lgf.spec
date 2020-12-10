<Geometry name="test1">
	<Meshes>
		<Mesh matId="0">
			<VerticeCount count="24"/>
			<IndiceCount count="36"/>
		</Mesh>
	</Meshes>
	<Materials>
		<Material name="">
			<Diffuse path="engine/textures/default/defaultTexture.png"/>
			<Specular path="engine/textures/default/defaultTexture.png"/>
			<Shininess value="32.000000"/>
			<Shader name="Illumn" vertex="engine/shaders/3d/shader_vs.glsl" fragment="engine/shaders/3d/shader_fs.glsl"/>
		</Material>
	</Materials>
	<BoundingBox>
		<Max position="1,1,2"/>
		<Min position="-1,-1,0"/>
	</BoundingBox>
</Geometry>

