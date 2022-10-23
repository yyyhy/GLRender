

#include"object.hpp"

class CameraController :public Component{
private:
	Transform* trans;
	GLFWwindow* window;
public:

	void SetWindow(GLFWwindow* w) {
		window = w;
	}

	CameraController() = default;

	void Update() override {
		if(trans==NULL)
			trans = object->GetComponent<Transform>();
		if (trans == NULL)
			std::cout << "CONTROLLER INIT FALSE" << "\n";
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			trans->Translate(0.1, 0, 0);
		}
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			trans->Translate(-0.1, 0, 0);
		}
		else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			trans->Translate(0, -0.1, 0);
		}
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			trans->Translate(0, 0.1, 0);
		}
	}
};