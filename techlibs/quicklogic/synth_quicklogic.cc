/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2020 Karol Gugala <kgugala@antmicro.com>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
#include "kernel/register.h"
#include "kernel/celltypes.h"
#include "kernel/rtlil.h"
#include "kernel/log.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct SynthQuickLogicPass : public ScriptPass {

    SynthQuickLogicPass() : ScriptPass("synth_quicklogic", "Synthesis for QuickLogic FPGAs") {}

    void help() override
    {
        log("\n");
        log("   synth_quicklogic [options]\n");
        log("This command runs synthesis for QuickLogic FPGAs\n");
        log("\n");
        log("    -top <module>\n");
        log("         use the specified module as top module\n");
        log("\n");
        log("    -family <family>\n");
        log("        run synthesis for the specified QuickLogic architecture\n");
        log("        generate the synthesis netlist for the specified family.\n");
        log("        supported values:\n");
        log("        - pp3: PolarPro 3 \n");
        log("        - ap3: ArcticPro 3 \n");
        log("\n");
        log("    -edif <file>\n");
        log("        write the design to the specified edif file. writing of an output file\n");
        log("        is omitted if this parameter is not specified.\n");
        log("\n");
        log("    -blif <file>\n");
        log("        write the design to the specified BLIF file. writing of an output file\n");
        log("        is omitted if this parameter is not specified.\n");
        log("\n");
        log("    -adder\n");
        log("        use adder cells in output netlist\n");
        log("\n");
        log("The following commands are executed by this synthesis command:\n");
        help_script();
        log("\n");
    }

    string top_opt, edif_file, blif_file, family, currmodule;
    bool inferAdder;

    void clear_flags() YS_OVERRIDE
    {
        top_opt = "-auto-top";
        edif_file = "";
        blif_file = "";
        currmodule = "";
        family = "pp3";
        inferAdder = false;
    }

    void execute(std::vector<std::string> args, RTLIL::Design *design) YS_OVERRIDE
    {
        string run_from, run_to;
        clear_flags();

	std::string top_opt, edif_file, blif_file, family, currmodule;
	bool inferAdder;

	void clear_flags() YS_OVERRIDE
	{
		top_opt = "-auto-top";
		edif_file = "";
		blif_file = "";
		currmodule = "";
		family = "pp3";
		inferAdder = false;
	}

	void execute(std::vector<std::string> args, RTLIL::Design *design) YS_OVERRIDE
	{
		std::string run_from, run_to;
		clear_flags();

		size_t argidx;
		for (argidx = 1; argidx < args.size(); argidx++)
		{
			if (args[argidx] == "-top" && argidx+1 < args.size()) {
				top_opt = "-top " + args[++argidx];
				continue;
			}
			if (args[argidx] == "-edif" && argidx+1 < args.size()) {
				edif_file = args[++argidx];
				continue;
			}
            if (args[argidx] == "-family" && argidx+1 < args.size()) {
                family = args[++argidx];
                continue;
            }
            if (args[argidx] == "-blif" && argidx+1 < args.size()) {
                blif_file = args[++argidx];
                continue;
            }
            if (args[argidx] == "-adder") {
                inferAdder = true;
                continue;
            }
            break;
        }
        extra_args(args, argidx, design);

        if (!design->full_selection())
            log_cmd_error("This command only operates on fully selected designs!\n");

        log_header(design, "Executing SYNTH_QUICKLOGIC pass.\n");
        log_push();

        run_script(design, run_from, run_to);

        log_pop();
    }

    void script() YS_OVERRIDE
    {
        if (check_label("begin")) {
            std::string readVelArgs = " +/quicklogic/" + family + "_cells_sim.v";
            run("read_verilog -lib -specify +/quicklogic/cells_sim.v" + readVelArgs);
            run(stringf("hierarchy -check %s", help_mode ? "-top <top>" : top_opt.c_str()));
        }

        if (check_label("prepare")) {
            run("proc");
            run("flatten");
            run("tribuf -logic");
            run("opt_expr");
            run("opt_clean");
            run("deminout");
            run("opt");
        }

        if (check_label("coarse")) {
            run("opt_expr");
            run("opt_clean");
            run("check");
            run("opt");
            run("wreduce -keepdc");
            run("peepopt");
            run("pmuxtree");
            run("opt_clean");
            run("share");
            run("techmap -map +/cmp2lut.v -D LUT_WIDTH=4");
            run("opt_expr");
            run("opt_clean");

            run("alumacc");
            run("opt");
            run("fsm");
            run("opt -fast");
<<<<<<< HEAD

            run("read_verilog -lib +/quicklogic/cells_sim.v");
||||||| parent of a1151941b (Adding lut optimization support for AP3)

            run("memory -nomap");
            run("opt_clean");
        }
=======
            run("memory -nomap");
            run("opt_clean");
        }
>>>>>>> a1151941b (Adding lut optimization support for AP3)

<<<<<<< HEAD
			run(stringf("hierarchy -check %s", help_mode ? "-top <top>" : top_opt.c_str()));
		}
||||||| parent of a1151941b (Adding lut optimization support for AP3)
        if (check_label("map_ffram"))
        {
            run("opt -fast -mux_undef -undriven -fine");
            run("memory_map");
            run("opt -undriven -fine");
        }
=======
        if (check_label("map_ffram")) {
            run("opt -fast -mux_undef -undriven -fine");
            run("memory_map -iattr -attr !ram_block -attr !rom_block -attr logic_block "
                    "-attr syn_ramstyle=auto -attr syn_ramstyle=registers "
                    "-attr syn_romstyle=auto -attr syn_romstyle=logic");
            run("opt -undriven -fine");
        }
>>>>>>> a1151941b (Adding lut optimization support for AP3)

<<<<<<< HEAD
		if (check_label("flatten"))
		{
			run("proc");
			run("flatten");
            run("wreduce -keepdc");
            run("muxpack");
			run("tribuf -logic");
			run("deminout");
			run("opt");
			run("opt_clean");
			run("peepopt");
		}

		if (check_label("coarse"))
		{
			run("synth -run coarse");
			/*run("opt");
			run("opt_clean");
			run("peepopt");
			run("techmap");
			run("opt");
			run("check");*/
		}

		if (check_label("map_ffram"))
		{
			run("opt -fast -mux_undef -undriven -fine");
			run("memory_map");
			run("opt -undriven -fine");
		}

		if (check_label("map_gates"))
		{
||||||| parent of a1151941b (Adding lut optimization support for AP3)
        if (check_label("map_gates"))
        {
=======
        if (check_label("map_gates")) {
>>>>>>> a1151941b (Adding lut optimization support for AP3)
            if (inferAdder)
            {
                run("techmap -map +/techmap.v -map +/quicklogic/" + family + "_arith_map.v");
            } else {
                run("techmap");
            }
            if (family == "pp3") {
                run("muxcover -mux8 -mux4");
            }
            run("opt_expr -clkinv");
            run("opt -fast");
            run("opt_expr");
            run("opt_merge");
            run("opt_rmdff");
            run("opt_clean");
            run("opt");
        }

        if (check_label("map_ffs")) {
            if (family == "pp3") {
                run("dff2dffe");
            } else {
                run("dff2dffe -direct-match $_DFF_*");
            }

            std::string techMapArgs = " -map +/quicklogic/" + family + "_ffs_map.v";
            run("techmap " + techMapArgs);
            run("opt_expr -mux_undef");
            run("simplemap");
            run("opt_expr");
            run("opt_merge");
            run("opt_rmdff");
            run("opt_clean");
            run("opt");
        }

        if (check_label("map_luts")) {
            std::string techMapArgs = " -map +/quicklogic/" + family + "_latches_map.v";
            run("techmap " + techMapArgs);
            if (family == "pp3") {
                run("abc -luts 1,2,2");
            } else {
                run("nlutmap -luts N_4");
                run("abc -dress -lut 4 -dff");
            }

            techMapArgs = " -map +/quicklogic/" + family + "_ffs_map.v";
            run("techmap " + techMapArgs);
            run("clean");

            techMapArgs = " -map +/quicklogic/" + family + "_lut_map.v";
            run("techmap " + techMapArgs);
            run("clean");
            run("opt_lut -dlogic carry:A=2:B=1:CI=0");
        }

        if (check_label("map_cells")) {

            std::string techMapArgs = " -map +/quicklogic/" + family + "_cells_map.v";
            run("techmap" + techMapArgs);
            run("clean");
        }

        if (check_label("check")) {
            run("autoname");
            run("hierarchy -check");
            run("stat");
            run("check -noinit");
        }

        if (check_label("iomap")) {
            if (family == "pp3") {
                run("clkbufmap -buf $_BUF_ Y:A -inpad ckpad Q:P");
                run("iopadmap -bits -outpad outpad A:P -inpad inpad Q:P -tinoutpad bipad EN:Q:A:P A:top");
            } else if (family == "ap3") {
                run("clkbufmap -buf $_BUF_ Y:A -inpad ck_buff Q:A");
                run("iopadmap -bits -outpad out_buff A:Q -inpad in_buff Q:A -toutpad EN:A:Q A:top");
            }
        }

        if (check_label("finalize")) {
            run("splitnets -ports -format ()");
            run("setundef -zero -params -undriven");
            run("hilomap -hicell logic_1 a -locell logic_0 a -singleton A:top");
            run("opt_clean");
            run("check");
            run("autoname");
			run("clean");
		}

        if (check_label("edif")) {
            if (!edif_file.empty() || help_mode)
                run(stringf("write_edif -nogndvcc -attrprop -pvector par %s %s", this->currmodule.c_str(), edif_file.c_str()));
        }

        if (check_label("blif")) {
            if (!blif_file.empty() || help_mode)
                run(stringf("write_blif %s %s", top_opt.c_str(), blif_file.c_str()));
        }

    }
} SynthQuicklogicPass;

PRIVATE_NAMESPACE_END
