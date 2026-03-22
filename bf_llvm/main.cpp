#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Host.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <vector>

using namespace std;

static void usage(string &prog_name) {
    cerr << "Usage: " << prog_name << " <input.bf> [-o output]\n";
    exit(1);
}

static string readFile(const string &path) {
    ifstream in(path, ios::in | ios::binary);
    if (!in) return {};

    string contents;
    in.seekg(0, ios::end);
    contents.resize(static_cast<size_t>(in.tellg()));
    in.seekg(0, ios::beg);
    in.read(contents.data(), static_cast<streamsize>(contents.size()));

    return contents;
}

static vector<char> filterOps(const std::string &source) {
    vector<char> ops;
    ops.reserve(source.size());
    for (char c : source) {
        switch (c) {
            case '>':
            case '<':
            case '-':
            case '+':
            case '[':
            case ']':
            case '.':
            case ',':
                ops.push_back(c);
                break;
        }
    }
    return ops;
}

int main(int argc, char **argv) {
    string prog_name = argv[0];
    if (argc < 2) {
        usage(prog_name);
    }

    string inputPath = argv[1];
    string outputPath = "a.out";

    for (int i = 2; i < argc; i++) {
        if (string(argv[i]) == "-o") {
            if (i + 1 < argc) {
                outputPath = argv[++i];
            } else {
                usage(prog_name);
            }
        }
    }

    const string source = readFile(inputPath);
    if (source.empty()) {
        cerr << "Failed to read input file: " << inputPath << "\n ";
        return 1;
    }
    const vector<char> ops = filterOps(source);

    llvm::LLVMContext context;
    unique_ptr<llvm::Module> module =
        make_unique<llvm::Module>("bf_module", context);

    llvm::IRBuilder<> builder(context);

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    const std::string triple = llvm::sys::getDefaultTargetTriple();
    cout << "Host target triple: " << triple << "\n";
    llvm::Triple host_triple(triple);
    module->setTargetTriple(host_triple);

    string targetError;
    const llvm::Target *target =
        llvm::TargetRegistry::lookupTarget(host_triple, targetError);
    if (!target) {
        cerr << "Target Lookup failed for target: " << triple
             << "due to : " << targetError << "\n";
        return 1;
    }

    llvm::TargetOptions targetOpts;
    unique_ptr<llvm::TargetMachine> targetMachine =
        unique_ptr<llvm::TargetMachine>(target->createTargetMachine(
            host_triple, "generic", "", targetOpts, nullopt));
    if (!targetMachine) {
        cerr << "Failed to create TargetMachine\n";
        return 1;
    }

    module->setDataLayout(targetMachine->createDataLayout());

    llvm::Type *i32Ty = builder.getInt32Ty();
    llvm::Type *i8Ty = builder.getInt8Ty();

    // for '.' int putchar(int c);
    llvm::FunctionType *putcharTy =
        llvm::FunctionType::get(i32Ty, {i32Ty}, false);
    // for ',' int getchar();
    llvm::FunctionType *getcharTy = llvm::FunctionType::get(i32Ty, {}, false);

    llvm::FunctionCallee putcharFn =
        module->getOrInsertFunction("putchar", putcharTy);
    llvm::FunctionCallee getcharFn =
        module->getOrInsertFunction("getchar", getcharTy);

    llvm::FunctionType *mainTy = llvm::FunctionType::get(i32Ty, {}, false);
    llvm::Function *mainFn = llvm::Function::Create(
        mainTy, llvm::Function::ExternalLinkage, "main", module.get());

    llvm::BasicBlock *entry =
        llvm::BasicBlock::Create(context, "entry", mainFn);
    builder.SetInsertPoint(entry);

    const int tapeSize = 30000;

    llvm::AllocaInst *tape =
        builder.CreateAlloca(i8Ty, builder.getInt32(tapeSize), "tape");
    llvm::AllocaInst *idx = builder.CreateAlloca(i32Ty, nullptr, "idx");
    builder.CreateStore(builder.getInt32(0), idx);

    builder.CreateMemSet(tape, builder.getInt8(0), builder.getInt64(tapeSize),
                         llvm::Align(1));

    // // new builder to force allocas at start of main
    // llvm::IRBuilder<> entryBuilder(entry, entry->begin());

    // // 30K memory i8*
    // llvm::AllocaInst *tape =
    //     entryBuilder.CreateAlloca(i8Ty, builder.getInt32(tapeSize), "tape");
    // // index into memory i32
    // llvm::AllocaInst *idx = entryBuilder.CreateAlloca(i32Ty, nullptr, "idx");

    // entryBuilder.CreateStore(builder.getInt32(0), idx);

    // entryBuilder.CreateMemSet(tape, builder.getInt8(0),
    //                           builder.getInt64(tapeSize), llvm::Align(1));

    auto cellPtr = [&]() -> llvm::Value * {
        llvm::Value *idxVal = builder.CreateLoad(i32Ty, idx, "idx");
        return builder.CreateInBoundsGEP(i8Ty, tape, idxVal, "cell.ptr");
    };

    struct LoopBlocks {
        llvm::BasicBlock *cond;
        llvm::BasicBlock *end;
    };
    stack<LoopBlocks> loopStack;

    for (char op : ops) {
        switch (op) {
            case '>': {
                llvm::Value *v = builder.CreateLoad(i32Ty, idx, "idx");
                llvm::Value *n =
                    builder.CreateAdd(v, builder.getInt32(1), "idx.inc");
                builder.CreateStore(n, idx);
            } break;
            case '<': {
                llvm::Value *v = builder.CreateLoad(i32Ty, idx, "idx");
                llvm::Value *n =
                    builder.CreateSub(v, builder.getInt32(1), "idx.dec");
                builder.CreateStore(n, idx);
            } break;
            case '.': {
                llvm::Value *ptr = cellPtr();
                llvm::Value *v = builder.CreateLoad(i8Ty, ptr, "cell");
                llvm::Value *z = builder.CreateZExt(v, i32Ty, "cell.zext");
                builder.CreateCall(putcharFn, {z});
            } break;
            case ',': {
                llvm::Value *c = builder.CreateCall(getcharFn, {}, "in");
                llvm::Value *t = builder.CreateTrunc(c, i8Ty, "in.trync");
                llvm::Value *ptr = cellPtr();
                builder.CreateStore(t, ptr);
            } break;
            case '+': {
                llvm::Value *ptr = cellPtr();
                llvm::Value *v = builder.CreateLoad(i8Ty, ptr, "cell");
                llvm::Value *n =
                    builder.CreateAdd(v, builder.getInt8(1), "cell.inc");
                builder.CreateStore(n, ptr);

            } break;
            case '-': {
                llvm::Value *ptr = cellPtr();
                llvm::Value *v = builder.CreateLoad(i8Ty, ptr, "cell");
                llvm::Value *n =
                    builder.CreateSub(v, builder.getInt8(1), "cell.dec");
                builder.CreateStore(n, ptr);

            } break;
            case '[': {
                // create basic blocks for cond -> body ->end
                llvm::BasicBlock *cond =
                    llvm::BasicBlock::Create(context, "loop.cond", mainFn);
                llvm::BasicBlock *body =
                    llvm::BasicBlock::Create(context, "loop.body", mainFn);
                llvm::BasicBlock *end =
                    llvm::BasicBlock::Create(context, "loop.end", mainFn);

                // jump to cond, then branch based on val in cellPtr
                builder.CreateBr(cond);
                builder.SetInsertPoint(cond);

                llvm::Value *ptr = cellPtr();
                llvm::Value *v = builder.CreateLoad(i8Ty, ptr, "cell");
                // jump if *cellPtr == 0
                llvm::Value *nz =
                    builder.CreateICmpNE(v, builder.getInt8(0), "cell.nz");
                builder.CreateCondBr(nz, body, end);

                builder.SetInsertPoint(body);
                loopStack.push({cond, end});
            } break;
            case ']': {
                if (loopStack.empty()) {
                    cerr << "Unmatched ']' in source\n";
                    return 1;
                }

                LoopBlocks loop = loopStack.top();
                loopStack.pop();

                // if body doesn't already terminate, jump back to cond;
                if (!builder.GetInsertBlock()->getTerminator()) {
                    builder.CreateBr(loop.cond);
                }

                builder.SetInsertPoint(loop.end);
            } break;
            default:
                cerr << "Unreachable\n";
                return 1;
        }
    }

    if (!loopStack.empty()) {
        cerr << "Unmatched ']' in source\n";
        return 1;
    }

    if (!builder.GetInsertBlock()->getTerminator()) {
        builder.CreateRet(builder.getInt32(0));
    }

    if (llvm::verifyModule(*module, &llvm::errs())) {
        cerr << "Module Verification failed\n";
        return 1;
    }
    string objPath = outputPath + ".o";
    error_code ec;
    llvm::raw_fd_ostream dest(objPath, ec, llvm::sys::fs::OF_None);

    if (ec) {
        cerr << "Failed to open obj file: " << objPath
             << " because: " << ec.message() << "\n";
        return 1;
    }

    llvm::legacy::PassManager pass;
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr,
                                           llvm::CodeGenFileType::ObjectFile)) {
        cerr << "TargetMachine cannot an obj file\n";
        return 1;
    }

    pass.run(*module);
    dest.flush();

    string llPath = outputPath + ".ll";
    error_code lec;
    llvm::raw_fd_ostream llout(llPath, lec, llvm::sys::fs::OF_None);
    if (lec) {
        cerr << "Failed ot open ll file: " << llPath
             << " because: " << lec.message() << "\n";
        return 1;
    }
    module->print(llout, nullptr);

    return 0;
}
