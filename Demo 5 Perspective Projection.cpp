#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

const char* vertexShaderSource = R"(
#version 330 core 
layout(location = 0) in vec3 aPos;  //输入顶点位置

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core 
out vec4 FragColor;  //输出颜色

void main()
{
	FragColor = vec4(1.0, 0.5, 0.2, 1.0);  //橙色
}
)";

int main()
{
	glfwInit();  //初始化GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  //OpenGL 主版本
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);	//次版本
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//创建窗口
	GLFWwindow* window = glfwCreateWindow(800, 600, "Triangle", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "创建窗口失败" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);  // 设置当前上下文

	//初始化GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "初始化 GLAD 失败" << std::endl;
		return -1;
	}

	//设置视口
	glViewport(0, 0, 800, 600);

	//====Shader 编译====

	//创建顶点着色器对象
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	//创建片元着色器对象
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	//创建着色器程序
	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	//删除Shader
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//====顶点数据====
	float vertices[] = {
		-0.5f, -0.5f, 0.0f,  //左下
		 0.5f, -0.5f, 0.0f,  //右下
		 0.0f,  0.5f, 0.0f   //顶部
	};

	unsigned int VBO, VAO;

	glGenVertexArrays(1, &VAO); //创建VAO
	glGenBuffers(1, &VBO); //创建VBO

	//绑定VAO
	glBindVertexArray(VAO);

	//绑定VBO并传数据
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//告诉GPU如何解析数据
	glVertexAttribPointer(
		0,  //location = 0
		3,  //每个顶点3个float
		GL_FLOAT,  //类型
		GL_FALSE,  //不归一化
		3 * sizeof(float),  //步长
		(void*)0  //偏移
	);

	glEnableVertexAttribArray(0);  //启用属性

	//====渲染循环====
	while (!glfwWindowShouldClose(window))
	{
		//输入(按ESC关闭)
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		//清屏（背景色）
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//使用Shader
		glUseProgram(shaderProgram);

		//====创建变换矩阵====

		//平移，右移
		glm::mat4 view = glm::mat4(1.0f);
		float time = glfwGetTime();
		view = glm::translate(view, glm::vec3(-sin(time), 0.0f, -3.0f));

		//旋转，随时间变化
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, sin(time) * 2.0f));
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));

		//投影，解决比例问题
		glm::mat4 projection = glm::perspective(
			glm::radians(45.0f),  //视野角度(FOV)
			800.0f / 600.0f,  //宽高比
			0.1f,  //近裁剪面
			100.0f  //远裁剪面
		);

		//合并矩阵
		unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
		unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
		unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		//绑定VAO
		glBindVertexArray(VAO);

		//画三角形
		glDrawArrays(GL_TRIANGLES, 0, 3);

		//交换缓冲区
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//====释放资源====
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	glfwTerminate();
	return 0;
}