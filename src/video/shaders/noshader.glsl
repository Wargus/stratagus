R"(
#if defined(VERTEX)
varying vec2 v_texCoord;
void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    v_texCoord = vec2(gl_MultiTexCoord0);
}
#elif defined(FRAGMENT)
varying vec2 v_texCoord;
uniform sampler2D tex0;
void main() {
    gl_FragColor = texture2D(tex0, v_texCoord);
}
#endif
)"
