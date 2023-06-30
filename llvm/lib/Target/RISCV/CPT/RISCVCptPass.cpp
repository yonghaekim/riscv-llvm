#include "RISCV.h"
#include "RISCVSubtarget.h"
#include "RISCVRegisterInfo.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "RISCVMachineFunctionInfo.h"
#include "MCTargetDesc/RISCVInstPrinter.h"

#include "llvm/CPT/CPT.h"

#define DEBUG_TYPE "RISCVCptPass"

STATISTIC(StatNumTagc,
            DEBUG_TYPE "Number of ETAGC intrinsics replaced");

STATISTIC(StatNumEact,
            DEBUG_TYPE "Number of EACT intrinsics replaced");

STATISTIC(StatNumEdea,
            DEBUG_TYPE "Number of EDEA intrinsics replaced");

STATISTIC(StatNumEchk,
            DEBUG_TYPE "Number of ECHK intrinsics replaced");

STATISTIC(StatNumEstr,
            DEBUG_TYPE "Number of ESTR intrinsics replaced");

STATISTIC(StatNumEclr,
            DEBUG_TYPE "Number of ECLR intrinsics replaced");

using namespace llvm;

namespace {
 class RISCVCptPass : public MachineFunctionPass {

 public:
   static char ID;

   RISCVCptPass() : MachineFunctionPass(ID) {}

   StringRef getPassName() const override { return DEBUG_TYPE; }

   virtual bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;

 private:
   const RISCVSubtarget *STI = nullptr;
   const RISCVInstrInfo *TII = nullptr;
   inline bool handleInstruction(MachineFunction &MF, MachineBasicBlock &MBB,
                                      MachineBasicBlock::instr_iterator &MIi);
  };
}

char RISCVCptPass::ID = 0;
FunctionPass *llvm::createRISCVCptPass() { return new RISCVCptPass(); }

bool RISCVCptPass::doInitialization(Module &M) {
  return true;
}

bool RISCVCptPass::runOnMachineFunction(MachineFunction &MF) {
  bool modified = false;
  STI = &MF.getSubtarget<RISCVSubtarget>();
  TII = STI->getInstrInfo();

  for (auto &MBB : MF) {
    for (auto MIi = MBB.instr_begin(), MIie = MBB.instr_end(); MIi != MIie;) {
			//(*MIi).dump();
      auto MIk = MIi++;

      switch (MIk->getOpcode()) {
        case RISCV::CPT_TAGC: {
          BuildMI(MBB, MIk, MIk->getDebugLoc(), TII->get(RISCV::TAGC), MIk->getOperand(0).getReg())
            .addReg(MIk->getOperand(1).getReg())
            .addReg(MIk->getOperand(2).getReg());

          MIk->removeFromParent();
          modified = true;
          ++StatNumTagc;

          break;
        }
        case RISCV::CPT_EACT: {
          BuildMI(MBB, MIk, MIk->getDebugLoc(), TII->get(RISCV::EACT))
            .addReg(MIk->getOperand(0).getReg())
            //.addReg(MIk->getOperand(1).getReg());
            .addImm(MIk->getOperand(1).getImm());

          MIk->removeFromParent();
          modified = true;
          ++StatNumEact;

          break;
        }
        case RISCV::CPT_EDEA: {
          BuildMI(MBB, MIk, MIk->getDebugLoc(), TII->get(RISCV::EDEA))
            .addReg(MIk->getOperand(0).getReg())
            //.addReg(MIk->getOperand(1).getReg());
            .addImm(MIk->getOperand(1).getImm());

          MIk->removeFromParent();
          modified = true;
          ++StatNumEact;

          break;
        }
        case RISCV::CPT_ECHK: {
          BuildMI(MBB, MIk, MIk->getDebugLoc(), TII->get(RISCV::ECHK))
            .addReg(MIk->getOperand(0).getReg())
            //.addReg(MIk->getOperand(1).getReg());
            .addImm(MIk->getOperand(1).getImm());

          MIk->removeFromParent();
          modified = true;
          ++StatNumEchk;

          break;
        }
        case RISCV::CPT_ESTR: {
          BuildMI(MBB, MIk, MIk->getDebugLoc(), TII->get(RISCV::ESTR))
            .addReg(MIk->getOperand(0).getReg())
            //.addReg(MIk->getOperand(1).getReg());
						.addImm(MIk->getOperand(1).getImm());
						//.addReg(RISCV::X0);
	
          MIk->removeFromParent();
          modified = true;
          ++StatNumEstr;

          break;
        }
        case RISCV::CPT_ECLR: {
          BuildMI(MBB, MIk, MIk->getDebugLoc(), TII->get(RISCV::ECLR))
            .addReg(MIk->getOperand(0).getReg())
            //.addReg(MIk->getOperand(1).getReg());
						.addImm(MIk->getOperand(1).getImm());
	
          MIk->removeFromParent();
          modified = true;
          ++StatNumEclr;

          break;
        }
        default:
          break;
      }
    }
  }

  return modified;
}

