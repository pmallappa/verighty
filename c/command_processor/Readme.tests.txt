How to write Tests
========================

Each test should contain following functions

1. the help function, 

	static void help(void)

2. the test_start function of the format of the format

	static int test_thread(cmd_t *cmd, conf_t *conf,
				 int argc, char *argv[])

   Test start function. at the command prompt
   =>
   by entering test_thread will start the test (calling the test_thread)
   function.
   cmd_t is the previously passed 'cmd_t test_cmd' structure.
   conf_t is the configuration
   argc, argv  are the standard args, can be used to pass arguments to tests
   for eg:
	=> test_thread -s 4 -n 3

3. Module initialization function

	int test_thread_register(void)
	{
		cmd_ops_t test_cmd_ops = {
			.func = test_thread,
			.help = help,
		};
	
		cmd_t test_cmd = {
			.name = "test_thread",
			.desc = "Run the app_slk thread",
			.ops = test_cmd_ops,
		};
	
		cmd_register(&test_cmd);
	
		return 0;
	}

   This registers the command and related functionality, command 
   to run test_thread to start this test.

4. The Initialization part (towards the end of file)

	INIT_FUNCTION(test_thread_register);

  This puths the module initialization routien in a separate section
  When the main function starts, it calls all initialization functions.

