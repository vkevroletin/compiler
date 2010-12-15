#include "generator.h" 

const string SIZE_TO_STR[] =
{
    "",
    "b",
    "s",
    "w",
    "l",
    "q"
};

const string REG_TO_STR[] =
{
    "%al",
    "%bl",
    "%cl",
    "%dl",
    "%ah",
    "%bh",
    "%ch",
    "%dh",
    "%ax",
    "%bx",
    "%cx",
    "%dx",
    "%dl",
    "%si",
    "%eax",
    "%ebx",
    "%ecx",
    "%edx",
    "%edi",
    "%esi",
    "%ebp",
    "%esp",
    "%st",
    "%st(0)",
    "%st(1)",
    "%st(2)",
    "%st(3)",
    "%st(4)",
    "%st(5)",
    "%st(6)",
    "%st(7)"
};

const string ASM_CMD_TO_STR[] =
{
    "add",
    "and",
    "call",
    "cmp",
    "div",
    "faddp",
    "fch",
    "fcompp",
    "fdivrp",
    "fild",
    "fld",
    "fmulp",
    "fnsts",
    "fstp",
    "fsubrp",
    "fxch",
    "idiv",
    "imul",
    "jmp",
    "jne",
    "jng",
    "jnl",
    "jnz",
    "jz",
    "lea",
    "mov",
    "movzb",
    "mul",
    "neg",
    "not",    
    "or",
    "pop",
    "push",
    "ret",
    "sahf",
    "sar",
    "sal",
    "seta",
    "setae",
    "setg",
    "setge",
    "setl",
    "setle",
    "sete",
    "setne",
    "sub",
    "test",
    "xor"
};

const string ASM_DATA_TYPE_TO_STR[] = 
{
    ".space",
    ".long",
    ".float",
    ".string"
};

//---AsmCmd---

void AsmCmd::Print(ostream& o) const
{
}

//---AsmLabel---

AsmLabel::AsmLabel(AsmImmidiate* label_):
    label(label_)
{
}

AsmLabel::AsmLabel(AsmImmidiate label_):
    label(new AsmImmidiate(label_))
{
}

AsmLabel::AsmLabel(string label):
    label(new AsmImmidiate(label))
{
}

void AsmLabel::Print(ostream& o) const
{
    o << "  ";
    label->PrintBase(o);
    o << ':';
}

//---AsmRawCmd---

AsmRawCmd::AsmRawCmd(string cmd):
    str(cmd)
{
}

void AsmRawCmd::Print(ostream& o) const
{
    o << str;
}

//---AsmCmd0---

AsmCmd0::AsmCmd0(AsmCmdName cmd, CmdSize cmd_size):
    command(cmd),
    size(cmd_size)
{
}

void AsmCmd0::Print(ostream& o) const
{
    o << "    " << ASM_CMD_TO_STR[command] << SIZE_TO_STR[size];
}

//---AsmData---

AsmData::AsmData(string name_, string value_, AsmDataType type_):
    name(name_),
    value(value_),
    type(type_)
{
}

void AsmData::Print(ostream& o) const
{
    o << "    " << name << ": " << ASM_DATA_TYPE_TO_STR[type] << ' ';
    if (type == DATA_STR) o << '\"' << value << '\"';
    else o << value;
}

//---AsmCmd1---

AsmCmd1::AsmCmd1(AsmCmdName cmd, AsmOperand* oper_, CmdSize size):
    AsmCmd0(cmd, size),
    oper(oper_)
{
}

void AsmCmd1::Print(ostream& o) const
{
    o << "    " << ASM_CMD_TO_STR[command] << SIZE_TO_STR[size] << '\t';
    if (size == SIZE_NONE)
        oper->PrintBase(o);
    else
        oper->Print(o);
}

//---AsmCmd2---

AsmCmd2::AsmCmd2(AsmCmdName cmd, AsmOperand* src_, AsmOperand* dest_, CmdSize size):
    AsmCmd0(cmd, size),
    src(src_),
    dest(dest_)
{
}

void AsmCmd2::Print(ostream& o) const
{
    o << "    " << ASM_CMD_TO_STR[command] << SIZE_TO_STR[size] << '\t';
    src->Print(o);
    o<< ", ";
    dest->Print(o);
}

//---AsmOperand---

void AsmOperand::Print(ostream& o) const
{
}

void AsmOperand::PrintBase(ostream& o) const
{
    Print(o);
}

//---AsmOperandBase---

void AsmOperandBase::Print(ostream& o) const
{
}

void AsmOperandBase::PrintBase(ostream& o) const
{
}

//---AsmRegister---

AsmRegister::AsmRegister(RegisterName reg_):
    reg(reg_)
{
}

void AsmRegister::Print(ostream& o) const
{
    o << REG_TO_STR[reg];
}

void AsmRegister::PrintBase(ostream& o) const
{
    o << REG_TO_STR[reg];
}

//---AsmImmidiate---

AsmImmidiate::AsmImmidiate()
{
}

AsmImmidiate::AsmImmidiate(const string& value_):
    value(value_)
{
}

AsmImmidiate::AsmImmidiate(int num)
{
    stringstream s;
    s << num;
    value += s.str();
}

AsmImmidiate::AsmImmidiate(const AsmImmidiate& src)
{
    value.assign(src.value);
}

void AsmImmidiate::Print(ostream& o) const
{
    o << '$' << value;
}

void AsmImmidiate::PrintBase(ostream& o) const
{
    o << value;
}


string AsmImmidiate::GetValue()
{
    return value;
}

//---AsmMemory---

AsmMemory::AsmMemory(AsmOperandBase* base_, int disp_, int index_, unsigned scale_):
    base(base_),
    disp(disp_),
    index(index_),
    scale(scale_)
{
}

AsmMemory::AsmMemory(AsmImmidiate base_, int disp_, int index_, unsigned scale_):
    base(new AsmImmidiate(base_)),
    disp(disp_),
    index(index_),
    scale(scale_)
{
}

AsmMemory::AsmMemory(RegisterName reg, int disp_, int index_, unsigned scale_):
    base(new AsmRegister(reg)),
    disp(disp_),
    index(index_),
    scale(scale_)
{
}

void AsmMemory::Print(ostream& o) const
{
    if (disp) o << disp;
    o << '(';
    base->PrintBase(o);
    if (index) o << ", " << index;
    if (scale) o << ", " << scale;
    o << ')';
}    

//---AsmCode---

AsmCode::AsmCode():
    funct_write(AsmImmidiate("printf")),
    label_counter(0),
    was_real(false),
    was_int(false),
    was_str(false),
    was_new_line(false)
{
}

AsmImmidiate AsmCode::GenLabel()
{
    return AsmImmidiate(label_counter++);
}

string AsmCode::GenStrLabel()
{
    stringstream s;
    s << label_counter++;
    return s.str();
}

AsmImmidiate AsmCode::GenLabel(string prefix)
{
    stringstream s;
    s << prefix << '_' << label_counter++;
    return AsmImmidiate(s.str());
}

string AsmCode::GenStrLabel(string prefix)
{
    stringstream s;
    s << prefix << '_' << label_counter++;
    return s.str();
}

AsmImmidiate AsmCode::LabelByStr(string str)
{
    return AsmImmidiate(ChangeName(str));
}

string AsmCode::ChangeName(string str)
{
    return str;
}

void AsmCode::AddCmd(AsmCmd* cmd)
{
    commands.push_back(cmd);
}

void AsmCode::AddCmd(AsmLabel label)
{
    commands.push_back(new AsmLabel(label));
}

void AsmCode::AddCmd(string raw_cmd)
{
    commands.push_back(new AsmRawCmd(raw_cmd));
}

void AsmCode::AddCmd(AsmCmdName cmd, CmdSize size)
{
    commands.push_back(new AsmCmd0(cmd, size));
}

void AsmCode::AddCmd(AsmCmdName cmd, AsmMemory mem, CmdSize size)
{
    commands.push_back(new AsmCmd1(cmd, new AsmMemory(mem), size));
}

void AsmCode::AddCmd(AsmCmdName cmd, RegisterName src, RegisterName dest, CmdSize size)
{
    commands.push_back(new AsmCmd2(cmd, new AsmRegister(src), new AsmRegister(dest), size));
}

void AsmCode::AddCmd(AsmCmdName cmd, AsmOperand* oper, CmdSize size)
{
    commands.push_back(new AsmCmd1(cmd, oper, size));
}

void AsmCode::AddCmd(AsmCmdName cmd, RegisterName reg, CmdSize size)
{
    commands.push_back(new AsmCmd1(cmd, new AsmRegister(reg), size));
}

void AsmCode::AddCmd(AsmCmdName cmd, AsmMemory* mem, CmdSize size)
{
    commands.push_back(new AsmCmd1(cmd, mem, size));
}

void AsmCode::AddCmd(AsmCmdName cmd, AsmImmidiate* imm, CmdSize size)
{
    commands.push_back(new AsmCmd1(cmd, imm, size));
}

void AsmCode::AddCmd(AsmCmdName cmd, AsmImmidiate imm, CmdSize size)
{
    AsmImmidiate* tmp = new AsmImmidiate(imm);
    commands.push_back(new AsmCmd1(cmd, tmp, size));
}

void AsmCode::AddCmd(AsmCmdName cmd, AsmOperand* oper1, AsmOperand* oper2, CmdSize size)
{
    commands.push_back(new AsmCmd2(cmd, oper1, oper2, size));
}
        
void AsmCode::AddCmd(AsmCmdName cmd, RegisterName reg, AsmImmidiate* dest, CmdSize size)
{
    commands.push_back(new AsmCmd2(cmd, new AsmRegister(reg), dest, size));
}

void AsmCode::AddCmd(AsmCmdName cmd, RegisterName reg, AsmImmidiate dest, CmdSize size)
{
    AsmImmidiate* imm = new AsmImmidiate(dest);
    commands.push_back(new AsmCmd2(cmd, new AsmRegister(reg), imm, size));
}

void AsmCode::AddCmd(AsmCmdName cmd, AsmImmidiate* src, RegisterName reg, CmdSize size)
{
    commands.push_back(new AsmCmd2(cmd, src, new AsmRegister(reg), size));
}

void AsmCode::AddCmd(AsmCmdName cmd, AsmImmidiate src, RegisterName reg, CmdSize size)
{
    AsmImmidiate* imm = new AsmImmidiate(src);
    commands.push_back(new AsmCmd2(cmd, imm, new AsmRegister(reg), size));
}

void AsmCode::AddCmd(AsmCmdName cmd, AsmMemory* mem, RegisterName reg, CmdSize size)
{
    commands.push_back(new AsmCmd2(cmd, mem, new AsmRegister(reg), size));
}

void AsmCode::AddCmd(AsmCmdName cmd, AsmMemory mem, RegisterName reg, CmdSize size)
{
    commands.push_back(new AsmCmd2(cmd, new AsmMemory(mem), new AsmRegister(reg), size));
}

void AsmCode::AddCmd(AsmCmdName cmd, RegisterName reg, AsmMemory* mem, CmdSize size)
{
    commands.push_back(new AsmCmd2(cmd, new AsmRegister(reg), mem, size));
}

void AsmCode::AddCmd(AsmCmdName cmd, RegisterName reg, AsmMemory mem, CmdSize size)
{
    commands.push_back(new AsmCmd2(cmd, new AsmRegister(reg), new AsmMemory(mem), size));
}

void AsmCode::AddCmd(AsmCmdName cmd, AsmImmidiate* src, AsmMemory* mem, CmdSize size)
{
    commands.push_back(new AsmCmd2(cmd, src, mem, size));
}

void AsmCode::AddCmd(AsmCmdName cmd, AsmImmidiate src, AsmMemory mem, CmdSize size)
{
    AddCmd(cmd, new AsmImmidiate(src), new AsmMemory(mem), size);
}

void AsmCode::AddData(AsmData* new_data)
{
    data.push_back(new_data);
}

AsmImmidiate AsmCode::AddData(string label, string value, AsmDataType type)
{
    string new_name = ChangeName(label); 
    data.push_back(new AsmData(new_name, value, type));
    return AsmImmidiate(new_name);
}

AsmImmidiate AsmCode::AddData(string label, unsigned size)
{
    string new_name = ChangeName(label);
    stringstream s;
    s << size;
    data.push_back(new AsmData(new_name, s.str()));
    return AsmImmidiate(new_name);
}

AsmImmidiate AsmCode::AddData(string value, AsmDataType type)
{
    return AddData(GenStrLabel(), value, type);
}

AsmImmidiate AsmCode::AddData(unsigned size)
{
    return AddData(GenStrLabel(), size);
}

void AsmCode::AddLabel(AsmImmidiate* label)
{
    commands.push_back(new AsmLabel(label));
}

void AsmCode::AddLabel(AsmImmidiate label)
{
    commands.push_back(new AsmLabel(label));
}

void AsmCode::AddLabel(string label)
{
    commands.push_back(new AsmLabel(label));
}

void AsmCode::Print(ostream& o) const
{
    o << ".data\n";
    for (vector<AsmData*>::const_iterator it = data.begin(); it != data.end(); ++it)
    {
        (*it)->Print(o);
        o << '\n';
    }
    o << ".text\n";
    for (vector<AsmCmd*>::const_iterator it = commands.begin(); it != commands.end(); ++it)
    {
        (*it)->Print(o);
        o << '\n';
    }
}

void AsmCode::GenCallWriteForInt()
{
    if (!was_int)
    {
        format_str_int = AddData(ChangeName("format_str_d"), "%d", DATA_STR);
        was_int = true;
    }
    AddCmd(ASM_PUSH, format_str_int);
    AddCmd(ASM_CALL, funct_write);
    AddCmd(ASM_ADD, AsmImmidiate(8), REG_ESP);
}

void AsmCode::GenCallWriteForReal()
{
    if (!was_real)
    {
        format_str_real = AddData(ChangeName("format_str_f"), "%f", DATA_STR);
        was_real = true;
    }
    AddCmd(ASM_FLD, AsmMemory(REG_ESP), SIZE_SHORT);
    AddCmd(ASM_SUB, AsmImmidiate(8), REG_ESP);
    AddCmd(ASM_FSTP, AsmMemory(REG_ESP, 4));
    AddCmd(ASM_MOV, format_str_real, AsmMemory(REG_ESP));
    AddCmd(ASM_CALL, funct_write);
    AddCmd(ASM_ADD, 12, REG_ESP);
}

void AsmCode::GenCallWriteForStr()
{
    if (!was_str)
    {
        format_str_str = AddData(ChangeName("format_str_s"), "%s", DATA_STR);
        was_str = true;
    }
    AddCmd(ASM_PUSH, format_str_str);
    AddCmd(ASM_CALL, funct_write);
    AddCmd(ASM_ADD, 8, REG_ESP);
}

void AsmCode::GenWriteNewLine()
{
    if (!was_new_line)
    {
        format_str_new_line = AddData(ChangeName("format_str_new_line"), "\\n", DATA_STR);
        was_new_line = true;
    }
    AddCmd(ASM_PUSH, format_str_new_line);
    AddCmd(ASM_CALL, funct_write);
    AddCmd(ASM_ADD, 4, REG_ESP);
}

void AsmCode::PushMemory(unsigned size)
{
    AddCmd(ASM_POP, REG_EBX);
    for (int i = 0; i < size; i += 4)
    {
        AddCmd(ASM_MOV, AsmMemory(REG_EBX, size - i - 4), REG_EAX);
        AddCmd(ASM_PUSH, REG_EAX);
    }
}

void AsmCode::MoveToMemoryFromStack(unsigned size)
{
    AddCmd(ASM_POP, REG_EBX);
    for (int i = 0; i < size; i += 4)
    {
        AddCmd(ASM_POP, REG_EAX);
        AddCmd(ASM_MOV, REG_EAX, AsmMemory(REG_EBX, i));
    }
}

void AsmCode::MoveMemory(unsigned size)
{
    AddCmd(ASM_POP, REG_EBX);
    AddCmd(ASM_POP, REG_EDX);
    AddCmd(ASM_SUB, size, REG_ESP);
    for (int i = 0; i < size; i += 4)
    {
        AddCmd(ASM_MOV, AsmMemory(REG_EBX, i), REG_EAX);
        AddCmd(ASM_MOV, REG_EAX, AsmMemory(REG_EDX, size - i - 4));
    }
}
