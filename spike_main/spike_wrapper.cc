#include "spike_wrapper.h"


#include "sim.h"
#include "mmu.h"
#include "remote_bitbang.h"
#include "extension.h"
#include <dlfcn.h>
#include <fesvr/option_parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <memory>
#include "../VERSION"

namespace spike_model
{    
    const char SpikeWrapper::name[] = "spike";

    SpikeWrapper::SpikeWrapper(sparta::TreeNode *node, const SpikeWrapperParameterSet *p) :
            sparta::Unit(node),
            p_(p->p),
            ic_(p->ic),
            dc_(p->dc),
            isa_(p->isa),
            cmd_(p->cmd),
            running_cores(std::stoul(p_))
    {
        sparta_assert(cmd_!="", "An application to simulate must be provided.");

        std::stringstream str_stream_cores;
        str_stream_cores << "-p" << p_;
        std::string param_num_cores = str_stream_cores.str();

        std::stringstream str_stream_isa;
        str_stream_isa << "--isa=" << isa_;
        std::string param_isa = str_stream_isa.str();
        
        std::stringstream str_stream_ic;
        str_stream_ic << "--ic=" << ic_;
        std::string param_ic = str_stream_ic.str();
        
        std::stringstream str_stream_dc;
        str_stream_dc << "--dc=" << dc_;
        std::string param_dc = str_stream_dc.str();

        std::vector<std::string> cmd_tokens;
        boost::split(cmd_tokens, cmd_, boost::is_any_of(" "), boost::token_compress_on);
    
        std::vector<std::string> args={"spike", param_num_cores, param_isa, param_ic, param_dc};

        args.insert(args.end(), cmd_tokens.begin(), cmd_tokens.end());


        //spike_thread=std::make_unique<std::thread>(spike_launcher, args, std::ref(spike));

        //spike.setupForInstructionLogging(args);
        setupForMissLogging(args); 
    }


    void SpikeWrapper::setupForMissLogging(std::vector<std::string> args)
    {
        setup(args);
        simulation->prepare();
    }


    void SpikeWrapper::setup(std::vector<std::string> args)
    {
        int argc=args.size();
        const char *argv[args.size()];
        for(int i=0;i<argc;i++)
        {
            argv[i]=args[i].c_str();
        }

        printf("This is Spike (SPARTA)!\n");
        printf("Inside with argc=%d, argv[1]: %s, argv[2]: %s, argv[3]:%s\n", argc, argv[1], argv[2], argv[3]);
        
        start_spike(argc, argv);
    }
            
    bool SpikeWrapper::simulateOne(uint16_t core, uint64_t current_cycle, std::list<std::shared_ptr<spike_model::L2Request>>& l1Misses)
    {
        return simulation->simulate_one(core, current_cycle, l1Misses);
    }

    bool SpikeWrapper::ackRegister(const spike_model::L2Request & req, uint64_t timestamp)
    {
        return simulation->ack_register(req, timestamp);
    }

    static void help(int exit_code = 1)
    {
      fprintf(stderr, "Spike RISC-V ISA Simulator " SPIKE_VERSION "\n\n");
      fprintf(stderr, "usage: spike [host options] <target program> [target options]\n");
      fprintf(stderr, "Host Options:\n");
      fprintf(stderr, "  -p<n>                 Simulate <n> processors [default 1]\n");
      fprintf(stderr, "  -m<n>                 Provide <n> MiB of target memory [default 2048]\n");
      fprintf(stderr, "  -m<a:m,b:n,...>       Provide memory regions of size m and n bytes\n");
      fprintf(stderr, "                          at base addresses a and b (with 4 KiB alignment)\n");
      fprintf(stderr, "  -d                    Interactive debug mode\n");
      fprintf(stderr, "  -g                    Track histogram of PCs\n");
      fprintf(stderr, "  -l                    Generate a log of execution\n");
      fprintf(stderr, "  -h, --help            Print this help message\n");
      fprintf(stderr, "  -H                    Start halted, allowing a debugger to connect\n");
      fprintf(stderr, "  --isa=<name>          RISC-V ISA string [default %s]\n", DEFAULT_ISA);
      fprintf(stderr, "  --priv=<m|mu|msu>     RISC-V privilege modes supported [default %s]\n", DEFAULT_PRIV);
      fprintf(stderr, "  --varch=<name>        RISC-V Vector uArch string [default %s]\n", DEFAULT_VARCH);
      fprintf(stderr, "  --pc=<address>        Override ELF entry point\n");
      fprintf(stderr, "  --hartids=<a,b,...>   Explicitly specify hartids, default is 0,1,...\n");
      fprintf(stderr, "  --ic=<S>:<W>:<B>      Instantiate a cache model with S sets,\n");
      fprintf(stderr, "  --dc=<S>:<W>:<B>        W ways, and B-byte blocks (with S and\n");
      fprintf(stderr, "  --l2=<S>:<W>:<B>        B both powers of 2).\n");
      fprintf(stderr, "  --device=<P,B,A>      Attach MMIO plugin device from an --extlib library\n");
      fprintf(stderr, "                          P -- Name of the MMIO plugin\n");
      fprintf(stderr, "                          B -- Base memory address of the device\n");
      fprintf(stderr, "                          A -- String arguments to pass to the plugin\n");
      fprintf(stderr, "                          This flag can be used multiple times.\n");
      fprintf(stderr, "                          The extlib flag for the library must come first.\n");
      fprintf(stderr, "  --log-cache-miss      Generate a log of cache miss\n");
      fprintf(stderr, "  --extension=<name>    Specify RoCC Extension\n");
      fprintf(stderr, "  --extlib=<name>       Shared library to load\n");
      fprintf(stderr, "                        This flag can be used multiple times.\n");
      fprintf(stderr, "  --rbb-port=<port>     Listen on <port> for remote bitbang connection\n");
      fprintf(stderr, "  --dump-dts            Print device tree string and exit\n");
      fprintf(stderr, "  --disable-dtb         Don't write the device tree blob into memory\n");
      fprintf(stderr, "  --dm-progsize=<words> Progsize for the debug module [default 2]\n");
      fprintf(stderr, "  --dm-sba=<bits>       Debug bus master supports up to "
          "<bits> wide accesses [default 0]\n");
      fprintf(stderr, "  --dm-auth             Debug module requires debugger to authenticate\n");
      fprintf(stderr, "  --dmi-rti=<n>         Number of Run-Test/Idle cycles "
          "required for a DMI access [default 0]\n");
      fprintf(stderr, "  --dm-abstract-rti=<n> Number of Run-Test/Idle cycles "
          "required for an abstract command to execute [default 0]\n");
      fprintf(stderr, "  --dm-no-hasel         Debug module supports hasel\n");
      fprintf(stderr, "  --dm-no-abstract-csr  Debug module won't support abstract to authenticate\n");
      fprintf(stderr, "  --dm-no-halt-groups   Debug module won't support halt groups\n");

      exit(exit_code);
    }

    static void suggest_help()
    {
      fprintf(stderr, "Try 'spike --help' for more information.\n");
      exit(1);
    }

    static std::vector<std::pair<reg_t, mem_t*>> make_mems(const char* arg)
    {
      // handle legacy mem argument
      char* p;
      auto mb = strtoull(arg, &p, 0);
      if (*p == 0) {
        reg_t size = reg_t(mb) << 20;
        if (size != (size_t)size)
          throw std::runtime_error("Size would overflow size_t");
        return std::vector<std::pair<reg_t, mem_t*>>(1, std::make_pair(reg_t(DRAM_BASE), new mem_t(size)));
      }

      // handle base/size tuples
      std::vector<std::pair<reg_t, mem_t*>> res;
      while (true) {
        auto base = strtoull(arg, &p, 0);
        if (!*p || *p != ':')
          help();
        auto size = strtoull(p + 1, &p, 0);
        if ((size | base) % PGSIZE != 0)
          help();
        res.push_back(std::make_pair(reg_t(base), new mem_t(size)));
        if (!*p)
          break;
        if (*p != ',')
          help();
        arg = p + 1;
      }
      return res;
    }
 
    void SpikeWrapper::start_spike(int argc, const char** argv)
    {
        bool debug = false;
        bool halted = false;
        bool histogram = false;
        bool log = false;
        bool dump_dts = false;
        bool dtb_enabled = true;
        size_t nprocs = 1;
        reg_t start_pc = reg_t(-1);
        std::vector<std::pair<reg_t, mem_t*>> mems;
        std::vector<std::pair<reg_t, abstract_device_t*>> plugin_devices;
        const char * ic_conf=NULL;
        const char * dc_conf=NULL;
        std::unique_ptr<cache_sim_t> l2;
        bool log_cache = false;
        bool log_commits = false;
        std::function<extension_t*()> extension;
        const char* isa = DEFAULT_ISA;
        const char* priv = DEFAULT_PRIV;
        const char* varch = DEFAULT_VARCH;
        uint16_t rbb_port = 0;
        bool use_rbb = false;
        unsigned dmi_rti = 0;
        debug_module_config_t dm_config = {
            .progbufsize = 2,
            .max_bus_master_bits = 0,
            .require_authentication = false,
            .abstract_rti = 0,
            .support_hasel = true,
            .support_abstract_csr_access = true,
            .support_haltgroups = true
        };
        std::vector<int> hartids;

        auto const hartids_parser = [&](const char *s) {
            std::string const str(s);
            std::stringstream stream(str);

            int n;
            while (stream >> n)
            {
                hartids.push_back(n);
                if (stream.peek() == ',') stream.ignore();
            }
        };

        auto const device_parser = [&plugin_devices](const char *s) {
            const std::string str(s);
            std::istringstream stream(str);

            // We are parsing a string like name,base,args.

            // Parse the name, which is simply all of the characters leading up to the
            // first comma. The validity of the plugin name will be checked later.
            std::string name;
            std::getline(stream, name, ',');
            if (name.empty()) {
                throw std::runtime_error("Plugin name is empty.");
            }

            // Parse the base address. First, get all of the characters up to the next
            // comma (or up to the end of the string if there is no comma). Then try to
            // parse that string as an integer according to the rules of strtoull. It
            // could be in decimal, hex, or octal. Fail if we were able to parse a
            // number but there were garbage characters after the valid number. We must
            // consume the entire string between the commas.
            std::string base_str;
            std::getline(stream, base_str, ',');
            if (base_str.empty()) {
                throw std::runtime_error("Device base address is empty.");
            }
            char* end;
            reg_t base = static_cast<reg_t>(strtoull(base_str.c_str(), &end, 0));
            if (end != &*base_str.cend()) {
                throw std::runtime_error("Error parsing device base address.");
            }

            // The remainder of the string is the arguments. We could use getline, but
            // that could ignore newline characters in the arguments. That should be
            // rare and discouraged, but handle it here anyway with this weird in_avail
            // technique. The arguments are optional, so if there were no arguments
            // specified we could end up with an empty string here. That's okay.
            auto avail = stream.rdbuf()->in_avail();
            std::string args(avail, '\0');
            stream.readsome(&args[0], avail);

            plugin_devices.emplace_back(base, new mmio_plugin_device_t(name, args));
        };

        option_parser_t parser;
        parser.help(&suggest_help);
        parser.option('h', "help", 0, [&](const char* s){help(0);});
        parser.option('d', 0, 0, [&](const char* s){debug = true;});
        parser.option('g', 0, 0, [&](const char* s){histogram = true;});
        parser.option('l', 0, 0, [&](const char* s){log = true;});
        parser.option('p', 0, 1, [&](const char* s){nprocs = atoi(s);});
        parser.option('m', 0, 1, [&](const char* s){mems = make_mems(s);});
        // I wanted to use --halted, but for some reason that doesn't work.
        parser.option('H', 0, 0, [&](const char* s){halted = true;});
        parser.option(0, "rbb-port", 1, [&](const char* s){use_rbb = true; rbb_port = atoi(s);});
        parser.option(0, "pc", 1, [&](const char* s){start_pc = strtoull(s, 0, 0);});
        parser.option(0, "hartids", 1, hartids_parser);
        parser.option(0, "ic", 1, [&](const char* s){ic_conf=s;});
        parser.option(0, "dc", 1, [&](const char* s){dc_conf=s;});
        parser.option(0, "l2", 1, [&](const char* s){l2.reset(cache_sim_t::construct(s, "L2$"));});
        parser.option(0, "log-cache-miss", 0, [&](const char* s){log_cache = true;});
        parser.option(0, "isa", 1, [&](const char* s){isa = s;});
        parser.option(0, "priv", 1, [&](const char* s){priv = s;});
        parser.option(0, "varch", 1, [&](const char* s){varch = s;});
        parser.option(0, "device", 1, device_parser);
        parser.option(0, "extension", 1, [&](const char* s){extension = find_extension(s);});
        parser.option(0, "dump-dts", 0, [&](const char *s){dump_dts = true;});
        parser.option(0, "disable-dtb", 0, [&](const char *s){dtb_enabled = false;});
        parser.option(0, "extlib", 1, [&](const char *s){
                void *lib = dlopen(s, RTLD_NOW | RTLD_GLOBAL);
                if (lib == NULL) {
                fprintf(stderr, "Unable to load extlib '%s': %s\n", s, dlerror());
                exit(-1);
                }
                });
        parser.option(0, "dm-progsize", 1,
                [&](const char* s){dm_config.progbufsize = atoi(s);});
        parser.option(0, "dm-sba", 1,
                [&](const char* s){dm_config.max_bus_master_bits = atoi(s);});
        parser.option(0, "dm-auth", 0,
                [&](const char* s){dm_config.require_authentication = true;});
        parser.option(0, "dmi-rti", 1,
                [&](const char* s){dmi_rti = atoi(s);});
        parser.option(0, "dm-abstract-rti", 1,
                [&](const char* s){dm_config.abstract_rti = atoi(s);});
        parser.option(0, "dm-no-hasel", 0,
                [&](const char* s){dm_config.support_hasel = false;});
        parser.option(0, "dm-no-abstract-csr", 0,
                [&](const char* s){dm_config.support_abstract_csr_access = false;});
        parser.option(0, "dm-no-halt-groups", 0,
                [&](const char* s){dm_config.support_haltgroups = false;});
        parser.option(0, "log-commits", 0, [&](const char* s){log_commits = true;});

        auto argv1 = parser.parse(argv);
        std::vector<std::string> htif_args(argv1, (const char*const*)argv + argc);
        if (mems.empty())
            mems = make_mems("2048");

        if (!*argv1)
            help();


        std::shared_ptr<sim_t> s=std::make_shared<sim_t> (isa, priv, varch, nprocs, halted, start_pc, mems, plugin_devices, htif_args,
                std::move(hartids), dm_config); //BORJA
        std::unique_ptr<remote_bitbang_t> remote_bitbang((remote_bitbang_t *) NULL);
        std::unique_ptr<jtag_dtm_t> jtag_dtm(
                new jtag_dtm_t(&s->debug_module, dmi_rti));
        if (use_rbb) {
            remote_bitbang.reset(new remote_bitbang_t(rbb_port, &(*jtag_dtm)));
            s->set_remote_bitbang(&(*remote_bitbang));
        }
        s->set_dtb_enabled(dtb_enabled);

        if (dump_dts) {
            printf("%s", s->get_dts());
            exit(0);
        }


      
        for (size_t i = 0; i < nprocs; i++)
        {
          if (ic_conf!=NULL)
          {
              icache_sim_t * ic;
              ic=new icache_sim_t(ic_conf);
              if(l2) ic->set_miss_handler(&*l2);
              ic->set_log(log_cache);
              s->get_core(i)->get_mmu()->register_memtracer(ic);
              ics.push_back(ic);
          }
          if (dc_conf!=NULL)
          {
              dcache_sim_t * dc;
              dc=new dcache_sim_t(dc_conf);
              if(l2) dc->set_miss_handler(&*l2);
              dc->set_log(log_cache);
              s->get_core(i)->get_mmu()->register_memtracer(dc);
              dcs.push_back(dc);
          }
          if (extension) s->get_core(i)->register_extension(extension());
        }

        s->set_debug(debug);
        s->set_log(log);
        s->set_histogram(histogram);
        s->set_log_commits(log_commits);

        simulation=s;
    }
}