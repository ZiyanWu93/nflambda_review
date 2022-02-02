class Command:
    def __init__(self, command_str, argument_list=[]):
        self.space = " "
        self.enter = "\n"
        self.command = command_str
        self.argument_list = argument_list

    def with_argument(self, argument_list):
        return Command(self.command, argument_list)

    def apply(self):
        result = self.command
        for arg in self.argument_list:
            result = result + self.space + arg
        # print(result)
        return (result + self.enter).encode()


command_config = Command("config")
command_mempool = Command("mempool")
command_em = Command("em")
command_nf = Command("nf")
command_worker = Command("worker")
command_port = Command("port")
command_dump = Command("dump")
command_coordinator = Command("coordinator")
command_core = Command("core")
command_action = Command("action")
command_state = Command("state")

