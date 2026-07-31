#include <stdio.h>
#include <vulkan/vulkan_core.h>

#include "headers/datatypes.h"
#include "headers/init.h"

int main() {
	Program app = {0};
	initialise(&app);

	printf("hello from main! :D\n");
	vkDestroyInstance(app.instance, NULL);
}
