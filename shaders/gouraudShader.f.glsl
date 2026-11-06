#version 410 core

// Gouraud Shading: lighting was already computed per vertex in the
// vertex shader. Here we just output the color the rasterizer
// interpolated across the triangle.

// Varying inputs

//interpolataed lit color
layout(location = 0) in vec4 vertexColor;

// Outputs

//color to apply to this fragment
out vec4 fragColorOut;

void main() {

    fragColorOut = vertexColor;

}//end main()