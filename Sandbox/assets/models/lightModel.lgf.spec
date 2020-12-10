<Geometry name="lightModel">
	<Meshes>
		<Mesh matId="0">
			<VerticeCount count="1984"/>
			<IndiceCount count="2880"/>
		</Mesh>
	</Meshes>
	<Materials>
		<Material name="">
			<Diffuse path="engine/textures/default/defaultTexture.png"/>
			<Specular path="engine/textures/default/defaultTexture.png"/>
			<Shininess value="1.000000"/>
			<Shader name="Line" vertex="engine/shaders/3d/lightShader_vs.glsl" fragment="engine/shaders/3d/lightShader_fs.glsl"/>
		</Material>
	</Materials>
	<BoundingBox>
		<Max position="0.5,0.5,0.5"/>
		<Min position="-0.5,-0.5,-0.5"/>
	</BoundingBox>
</Geometry>

