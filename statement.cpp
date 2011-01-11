#include "statement.h"

StmtAssign::StmtAssign(SyntaxNode* left_, SyntaxNode* right_):
    left(left_),
    right(right_)
{
}

const SyntaxNode* StmtAssign::GetLeft() const
{
    return left;
}

const SyntaxNode* StmtAssign::GetRight() const
{
    return right;
}

void StmtAssign::Print(ostream& o, int offset)
{
    PrintSpaces(o, offset) << ":= \n";
    left->Print(o, offset + 1);
    right->Print(o, offset + 1);
}

void StmtAssign::Print(ostream& o, int offset) const
{
    PrintSpaces(o, offset) << ":= \n";
    left->Print(o, offset + 1);
    right->Print(o, offset + 1);
}

void StmtAssign::Generate(AsmCode& asm_code)
{
    right->GenerateValue(asm_code);
    left->GenerateLValue(asm_code);
    asm_code.MoveToMemoryFromStack(left->GetSymType()->GetSize());
}

bool StmtAssign::IsAffectToVar(SymVar* var)
{
    return (left->GetAffectedVar() == var) || left->IsAffectToVar(var) || right->IsAffectToVar(var);
}

bool StmtAssign::IsDependOnVar(SymVar* var)
{
    return left->IsDependOnVar(var) || right->IsDependOnVar(var);
}

bool StmtAssign::IsHaveSideEffect()
{
    return (left->IsHaveSideEffect()) || (left->GetAffectedVar()->GetClassName() & SYM_VAR_GLOBAL)
        || right->IsHaveSideEffect();
}

void StmtAssign::GetAllAffectedVars(VarsContainer& res_cont)
{
    right->GetAllAffectedVars(res_cont);
    left->GetAllAffectedVars(res_cont);
    res_cont.insert(left->GetAffectedVar());
}

void StmtAssign::GetAllDependences(VarsContainer& res_cont, bool with_self)
{
    right->GetAllDependences(res_cont);
    left->GetAllDependences(res_cont, false);
}

StmtClassName StmtAssign::GetClassName() const
{
    return STMT_ASSIGN;
} 

void StmtAssign::MakeDependencesGraph(DependedVerts& v, DependencyGraph& g)
{
    left->MakeDependencesGraph(v, g);
    right->MakeDependencesGraph(v, g);
    AddToDependencyGraph(v, g, left->GetAffectedVar(), right);
}

//---StmtBlock---

void StmtBlock::Optimize()
{
    OptimizeLoops();
    OptimizeConstants();
    for (std::vector<NodeStatement*>::iterator it= statements.begin(); it != statements.end(); ++it)
        (*it)->Optimize();
}

void StmtBlock::OptimizeLoops()
{
    std::vector<NodeStatement*> optimized_body;
    for (std::vector<NodeStatement*>::iterator it= statements.begin(); it != statements.end(); ++it)
        if ((*it)->GetClassName() == STMT_LOOP)
        {
            StmtLoop* loop = (StmtLoop*)*it;
            loop->TakeOutVars(optimized_body);
            if (loop->GetBody()->GetSize() || loop->IsConditionAffectToVars()) optimized_body.push_back(loop);
        }
        else
        {
            optimized_body.push_back(*it);
        }
    statements.assign(optimized_body.begin(), optimized_body.end());
}

void StmtBlock::OptimizeConstants()
{
}

NodeStatement* StmtBlock::GetStmt(unsigned i)
{
    return statements[i];
}

bool StmtBlock::IsEmpty() const
{
    return statements.empty();
}

unsigned StmtBlock::GetSize() const
{
    return statements.size();
}

void StmtBlock::AddStatement(NodeStatement* new_stmt)
{
    if (new_stmt == NULL) return;
    if (new_stmt->GetClassName() == STMT_BLOCK)
        CopyContent((StmtBlock*)new_stmt);
    else
        statements.push_back(new_stmt);
}

void StmtBlock::CopyContent(StmtBlock* src)
{
    for (std::vector<NodeStatement*>::iterator it = src->statements.begin(); it != src->statements.end(); ++it)
      AddStatement(*it);
}

void StmtBlock::Print(ostream& o, int offset)
{
    PrintSpaces(o, offset) << "begin\n";
    for (vector<NodeStatement*>::const_iterator it = statements.begin(); it != statements.end(); ++it)
    {
//        (*it)->Print(o, offset + 1); continue;
        NodeStatement* node = *it;
        PrintSpaces(o, offset + 1) << "{se: " << node->IsHaveSideEffect();
        std::set<SymVar*> t;
        node->GetAllAffectedVars(t);
        if (!t.empty()) o << "; av: ";
        for (std::set<SymVar*>::iterator i = t.begin(); i != t.end(); ++i)
        {
            if (i != t.begin()) o << ", ";
            o << (*i)->GetToken().GetName();
        }
        node->GetAllDependences(t);
        if (!t.empty()) o << "; dp: ";
        for (std::set<SymVar*>::iterator i = t.begin(); i != t.end(); ++i)
        {
            if (i != t.begin()) o << ", ";
            o << (*i)->GetToken().GetName();
        }        
        o << "}\n";
        (*it)->Print(o, offset + 1);
    }
    PrintSpaces(o, offset) << "end\n";
}

void StmtBlock::Print(ostream& o, int offset) const
{
    PrintSpaces(o, offset) << "begin\n";
    for (vector<NodeStatement*>::const_iterator it = statements.begin(); it != statements.end(); ++it)
        (*it)->Print(o, offset + 1);
    PrintSpaces(o, offset) << "end\n";
}

void StmtBlock::Generate(AsmCode& asm_code)
{
    for (vector<NodeStatement*>::const_iterator it = statements.begin(); it != statements.end(); ++it)
        (*it)->Generate(asm_code);
}

bool StmtBlock::IsAffectToVar(SymVar* var) const
{
    for (std::vector<NodeStatement*>::const_iterator it = statements.begin(); it != statements.end(); ++it)
        if ((*it)->IsAffectToVar(var)) return true;
    return false;
}

bool StmtBlock::IsHaveSideEffect() const
{
    for (std::vector<NodeStatement*>::const_iterator it = statements.begin(); it != statements.end(); ++it)
        if ((*it)->IsHaveSideEffect()) return true;
    return false;
}

bool StmtBlock::IsAffectToVar(SymVar* var)
{
    for (std::vector<NodeStatement*>::iterator it = statements.begin(); it != statements.end(); ++it)
        if ((*it)->IsAffectToVar(var)) return true;
    return false;
}

bool StmtBlock::IsDependOnVar(SymVar* var)
{
    for (std::vector<NodeStatement*>::iterator it = statements.begin(); it != statements.end(); ++it)
        if ((*it)->IsDependOnVar(var)) return true;
    return false;
}

bool StmtBlock::IsHaveSideEffect()
{
    for (std::vector<NodeStatement*>::iterator it = statements.begin(); it != statements.end(); ++it)
        if ((*it)->IsHaveSideEffect()) return true;
    return false;
}

void StmtBlock::GetAllAffectedVars(VarsContainer& res_cont)
{
    for (std::vector<NodeStatement*>::iterator it = statements.begin(); it != statements.end(); ++it)
        ((*it)->GetAllAffectedVars(res_cont));
}

void StmtBlock::GetAllDependences(VarsContainer& res_cont, bool with_self)
{
    for (std::vector<NodeStatement*>::iterator it = statements.begin(); it != statements.end(); ++it)
        (*it)->GetAllDependences(res_cont);
}

void StmtBlock::MakeDependencesGraph(DependedVerts& v, DependencyGraph& g)
{
    for (std::vector<NodeStatement*>::iterator it = statements.begin(); it != statements.end(); ++it)
        (*it)->MakeDependencesGraph(v, g);
}

StmtClassName StmtBlock::GetClassName() const
{
    return STMT_BLOCK;
} 

//---StmtExpression---

StmtExpression::StmtExpression(SyntaxNode* expression):
    expr(expression)
{
}

void StmtExpression::Print(ostream& o, int offset)
{
    expr->Print(o, offset);
}

void StmtExpression::Print(ostream& o, int offset) const
{
    expr->Print(o, offset);
}

void StmtExpression::Generate(AsmCode& asm_code)
{
    expr->GenerateValue(asm_code);
    asm_code.AddCmd(ASM_ADD, expr->GetSymType()->GetSize(), REG_ESP);
}

bool StmtExpression::IsAffectToVar(SymVar* var)
{
    return expr->IsAffectToVar(var);
}

bool StmtExpression::IsDependOnVar(SymVar* var)
{
    return expr->IsDependOnVar(var);
}

bool StmtExpression::IsHaveSideEffect()
{
    return expr->IsHaveSideEffect();
}

void StmtExpression::GetAllAffectedVars(VarsContainer& res_cont)
{
    expr->GetAllAffectedVars(res_cont);
}

void StmtExpression::GetAllDependences(VarsContainer& res_cont, bool with_self)
{
    expr->GetAllDependences(res_cont);
}

void StmtExpression::MakeDependencesGraph(DependedVerts& v, DependencyGraph& g)
{
    expr->MakeDependencesGraph(v, g);
}

StmtClassName StmtExpression::GetClassName() const
{
    return STMT_EXPRESSION;
} 

bool StmtExpression::CanBeReplaced()
{
    return expr->CanBeReplaced();
}

//---StmtLoop---

bool StmtLoop::IsConditionAffectToVars()
{
    return true;
}

StmtBlock* StmtLoop::GetBody() const
{
    return body;
}

void StmtLoop::ObtainLabels(AsmCode& asm_code)
{
    break_label = asm_code.GenLabel("break");
    continue_label = asm_code.GenLabel("continue");;
}

void StmtLoop::CalculateDependences(set<SymVar*>& affected_cont, set<SymVar*>& deps)
{
}

void StmtLoop::TakeOutVars(std::vector<NodeStatement*>& before_loop)
{
    body->OptimizeLoops();
    set<SymVar*> affected_vars;
    set<SymVar*> dependences;
    CalculateDependences(affected_vars, dependences);
    for (int i = 0; i < body->GetSize(); ++i)
    {
        GetAllAffectedVars(affected_vars);
        GetAllDependences(dependences);
    }
    StmtBlock* new_body = new StmtBlock();
    for (int i = 0; i < body->GetSize(); ++i)
    {
        bool fixed = !body->GetStmt(i)->CanBeReplaced();
        fixed |= body->GetStmt(i)->IsDependOnVars(affected_vars);
        fixed |= body->GetStmt(i)->IsAffectToVars(dependences);
        if (fixed)
            new_body->AddStatement(body->GetStmt(i));
        else
            before_loop.push_back(body->GetStmt(i));
    }
    delete body;
    body = new_body;    
}

StmtLoop::StmtLoop(NodeStatement* body_)
{
    AddBody(body_);
}

AsmStrImmediate StmtLoop::GetBreakLabel() const
{
    return break_label;
}

AsmStrImmediate StmtLoop::GetContinueLabel() const
{
    return continue_label;
}

void StmtLoop::AddBody(NodeStatement* body_)
{
    if (body_ == NULL) return;
    if (body_->GetClassName() == STMT_BLOCK) body = (StmtBlock*)body_;
    else
    {
        body = new StmtBlock();
        body->AddStatement(body_);
    }
}

StmtClassName StmtLoop::GetClassName() const
{
    return STMT_LOOP;
} 

//---StmtFor---

void StmtFor::CalculateDependences(set<SymVar*>& affected_cont, set<SymVar*>& deps)
{
    init_val->GetAllAffectedVars(affected_cont);
    last_val->GetAllAffectedVars(affected_cont);
    init_val->GetAllDependences(deps);
    last_val->GetAllDependences(deps);    
    affected_cont.insert(index);
}

bool StmtFor::IsConditionAffectToVars()
{
    set<SymVar* > t;
    init_val->GetAllAffectedVars(t);
    last_val->GetAllAffectedVars(t);
    return !t.empty();
}

StmtFor::StmtFor(SymVar* index_, SyntaxNode* init_value, SyntaxNode* last_value, bool is_inc, NodeStatement* body_):
    StmtLoop(body_),
    index(index_),
    init_val(init_value),
    last_val(last_value),
    inc(is_inc)
{
}

void StmtFor::Print(ostream& o, int offset)
{
    PrintSpaces(o, offset) << "for " << (inc ? "to \n" : "downto \n");
    index->PrintAsNode(o, offset + 1);
    init_val->Print(o, offset + 1);
    last_val->Print(o, offset + 1);
    body->Print(o, offset);
}

void StmtFor::Print(ostream& o, int offset) const
{
    PrintSpaces(o, offset) << "for " << (inc ? "to \n" : "downto \n");
    index->PrintAsNode(o, offset + 1);
    init_val->Print(o, offset + 1);
    last_val->Print(o, offset + 1);
    body->Print(o, offset);
}

void StmtFor::Generate(AsmCode& asm_code)
{
    init_val->GenerateValue(asm_code);
    index->GenerateLValue(asm_code);
    asm_code.AddCmd(ASM_POP, REG_EAX);
    asm_code.AddCmd(ASM_POP, REG_EBX);
    asm_code.AddCmd(ASM_MOV, REG_EBX, AsmMemory(REG_EAX));
    AsmStrImmediate start_label(asm_code.GenLabel("for_check"));
    ObtainLabels(asm_code);
    last_val->GenerateValue(asm_code);
    asm_code.AddCmd(ASM_JMP, continue_label, SIZE_NONE);
    asm_code.AddLabel(start_label);
    body->Generate(asm_code);
    index->GenerateLValue(asm_code);
    asm_code.AddCmd(ASM_POP, REG_EAX);
    if (inc) asm_code.AddCmd(ASM_ADD, 1, AsmMemory(REG_EAX));
    else asm_code.AddCmd(ASM_SUB, 1, AsmMemory(REG_EAX));
    asm_code.AddLabel(continue_label);
    index->GenerateValue(asm_code);
    asm_code.AddCmd(ASM_POP, REG_EAX);
    asm_code.AddCmd(ASM_CMP, REG_EAX, AsmMemory(REG_ESP));
    if (inc) asm_code.AddCmd(ASM_JNL, start_label, SIZE_NONE);
    else asm_code.AddCmd(ASM_JNG, start_label, SIZE_NONE);
    asm_code.AddLabel(break_label);
    asm_code.AddCmd(ASM_ADD, 4, REG_ESP);
}

bool StmtFor::IsAffectToVar(SymVar* var)
{
    return (index == var) || body->IsAffectToVar(var) || init_val->IsAffectToVar(var)
        || last_val->IsAffectToVar(var);
}

bool StmtFor::IsDependOnVar(SymVar* var)
{
    return body->IsDependOnVar(var) || init_val->IsDependOnVar(var)
        || last_val->IsAffectToVar(var);
}

bool StmtFor::IsHaveSideEffect()
{
    return (index->GetClassName() & SYM_VAR_GLOBAL) || body->IsHaveSideEffect()
        || init_val->IsHaveSideEffect() || last_val->IsHaveSideEffect();
}

void StmtFor::GetAllAffectedVars(VarsContainer& res_cont)
{
    res_cont.insert(index);
    body->GetAllAffectedVars(res_cont);
    init_val->GetAllAffectedVars(res_cont);
    last_val->GetAllAffectedVars(res_cont);
}

void StmtFor::GetAllDependences(VarsContainer& res_cont, bool with_self)
{
    res_cont.insert(index);
    body->GetAllDependences(res_cont);
    init_val->GetAllDependences(res_cont);
    last_val->GetAllDependences(res_cont);
}

void StmtFor::MakeDependencesGraph(DependedVerts& v, DependencyGraph& g)
{
    AddToDependencyGraph(v, g, index, init_val);
    AddToDependencyGraph(v, g, index, last_val);   
    body->MakeDependencesGraph(v, g);
    init_val->MakeDependencesGraph(v, g);
    last_val->MakeDependencesGraph(v, g);
}

//---StmtWhile---

bool StmtWhile::IsConditionAffectToVars()
{
    set<SymVar*> t;
    condition->GetAllAffectedVars(t);
    return !t.empty();
}

void StmtWhile::CalculateDependences(set<SymVar*>& affected_cont, set<SymVar*>& deps)
{
    condition->GetAllAffectedVars(affected_cont);
    condition->GetAllDependences(deps);    
}

StmtWhile::StmtWhile(SyntaxNode* condition_, NodeStatement* body_):
    StmtLoop(body_),
    condition(condition_)
{
}

void StmtWhile::Print(ostream& o, int offset)
{
    PrintSpaces(o, offset) << "while\n";
    condition->Print(o, offset + 1);
    body->Print(o, offset + 1);
}

void StmtWhile::Print(ostream& o, int offset) const
{
    PrintSpaces(o, offset) << "while\n";
    condition->Print(o, offset + 1);
    body->Print(o, offset + 1);
}

void StmtWhile::Generate(AsmCode& asm_code)
{
    ObtainLabels(asm_code);
    asm_code.AddLabel(continue_label);
    condition->GenerateValue(asm_code);
    asm_code.AddCmd(ASM_POP, REG_EAX);
    asm_code.AddCmd(ASM_TEST, REG_EAX, REG_EAX);
    asm_code.AddCmd(ASM_JZ, break_label, SIZE_NONE);
    body->Generate(asm_code);
    asm_code.AddCmd(ASM_JMP, continue_label, SIZE_NONE);
    asm_code.AddLabel(break_label);
}

bool StmtWhile::IsAffectToVar(SymVar* var)
{
    return condition->IsAffectToVar(var) || body->IsAffectToVar(var);
}

bool StmtWhile::IsDependOnVar(SymVar* var)
{
    return condition->IsDependOnVar(var) || body->IsDependOnVar(var);
}

bool StmtWhile::IsHaveSideEffect()
{
    return condition->IsHaveSideEffect() || body->IsHaveSideEffect();
}

void StmtWhile::GetAllAffectedVars(VarsContainer& res_cont)
{
    condition->GetAllAffectedVars(res_cont);
    body->GetAllAffectedVars(res_cont);
}

void StmtWhile::GetAllDependences(VarsContainer& res_cont, bool with_selfcont)
{
    condition->GetAllDependences(res_cont);
    body->GetAllDependences(res_cont);
}

void StmtWhile::MakeDependencesGraph(DependedVerts& v, DependencyGraph& g)
{
    condition->MakeDependencesGraph(v, g);
    body->MakeDependencesGraph(v, g);
}

//---StmtUntil---

StmtUntil::StmtUntil(SyntaxNode* condition_, NodeStatement* body_):
    StmtWhile(condition, body_)
{
}

void StmtUntil::AddCondition(SyntaxNode* condition_)
{
    condition = condition_;
}

void StmtUntil::Print(ostream& o, int offset)
{
    PrintSpaces(o, offset) << "until\n";
    condition->Print(o, offset + 1);
    body->Print(o, offset + 1);
}

void StmtUntil::Print(ostream& o, int offset) const
{
    PrintSpaces(o, offset) << "until\n";
    condition->Print(o, offset + 1);
    body->Print(o, offset + 1);
}

void StmtUntil::Generate(AsmCode& asm_code)
{
    ObtainLabels(asm_code);
    AsmStrImmediate start_label(asm_code.GenLabel("until_start"));
    asm_code.AddLabel(start_label);
    body->Generate(asm_code);
    asm_code.AddLabel(continue_label);
    condition->GenerateValue(asm_code);
    asm_code.AddCmd(ASM_POP, REG_EAX);
    asm_code.AddCmd(ASM_TEST, REG_EAX, REG_EAX);
    asm_code.AddCmd(ASM_JZ, start_label, SIZE_NONE);
    asm_code.AddLabel(break_label);
}

//---StmtIf---

StmtIf::StmtIf(SyntaxNode* condition_, NodeStatement* then_branch_, NodeStatement* else_branch_):
    condition(condition_),
    then_branch(then_branch_),
    else_branch(else_branch_)
{
}

void StmtIf::Print(ostream& o, int offset)
{
    PrintSpaces(o, offset) << "if\n";
    condition->Print(o, offset + 1);
    PrintSpaces(o, offset) << "then\n";
    then_branch->Print(o, offset + 1);
    if (else_branch != NULL)
    {
        PrintSpaces(o, offset) << "else\n";
        else_branch->Print(o, offset + 1);
    }
}

void StmtIf::Print(ostream& o, int offset) const
{
    PrintSpaces(o, offset) << "if\n";
    condition->Print(o, offset + 1);
    PrintSpaces(o, offset) << "then\n";
    then_branch->Print(o, offset + 1);
    if (else_branch != NULL)
    {
        PrintSpaces(o, offset) << "else\n";
        else_branch->Print(o, offset + 1);
    }
}

void StmtIf::Generate(AsmCode& asm_code)
{
    bool is_const = condition->IsConst();
    bool is_true;
    if (is_const) is_true = condition->ComputeIntConstExpr();
    AsmStrImmediate label_else(asm_code.GenLabel("else"));
    AsmStrImmediate label_fin(asm_code.GenLabel("fin"));
    if (!is_const)
    {
        condition->GenerateValue(asm_code);
        asm_code.AddCmd(ASM_POP, REG_EAX);
        asm_code.AddCmd(ASM_TEST, REG_EAX, REG_EAX);
        asm_code.AddCmd(ASM_JZ, label_else, SIZE_NONE);
    }
    if (!is_const || is_true) then_branch->Generate(asm_code);
    if (!is_const)
    {
        asm_code.AddCmd(ASM_JMP, label_fin, SIZE_NONE);
        asm_code.AddLabel(label_else);
    }
    if (else_branch != NULL && (!is_const || !is_true)) else_branch->Generate(asm_code);
    if (!is_const)
    {
        asm_code.AddCmd(ASM_JMP, label_fin, SIZE_NONE);
        asm_code.AddLabel(label_fin);
    }
}

bool StmtIf::IsAffectToVar(SymVar* var)
{
    return condition->IsAffectToVar(var) ||
        (then_branch != NULL && then_branch->IsAffectToVar(var)) ||
        (else_branch != NULL && else_branch->IsAffectToVar(var));
}

bool StmtIf::IsDependOnVar(SymVar* var)
{
    return condition->IsDependOnVar(var) ||
        (then_branch != NULL && then_branch->IsDependOnVar(var)) ||
        (else_branch != NULL && else_branch->IsDependOnVar(var));    
}

bool StmtIf::IsHaveSideEffect()
{
    return condition->IsHaveSideEffect() ||
        (then_branch != NULL && then_branch->IsHaveSideEffect()) ||
        (else_branch != NULL && else_branch->IsHaveSideEffect());
}

void StmtIf::GetAllAffectedVars(VarsContainer& res_cont)
{
    condition->GetAllAffectedVars(res_cont);
    if (then_branch != NULL) then_branch->GetAllAffectedVars(res_cont);
    if (else_branch != NULL) else_branch->GetAllAffectedVars(res_cont);
}

void StmtIf::GetAllDependences(VarsContainer& res_cont, bool with_self)
{
    condition->GetAllDependences(res_cont);
    if (then_branch != NULL) then_branch->GetAllDependences(res_cont);
    if (else_branch != NULL) else_branch->GetAllDependences(res_cont);
}

void StmtIf::MakeDependencesGraph(DependedVerts& v, DependencyGraph& g)
{
    condition->MakeDependencesGraph(v, g);
    then_branch->MakeDependencesGraph(v, g);
    else_branch->MakeDependencesGraph(v, g);
}

StmtClassName StmtIf::GetClassName() const
{
    return STMT_IF;
} 

bool StmtIf::CanBeReplaced()
{
    return (then_branch == NULL) || then_branch->CanBeReplaced() &&
        (else_branch == NULL || else_branch->CanBeReplaced());    
}

//---StmtJump---

StmtJump::StmtJump(Token tok, StmtLoop* loop_):
    loop(loop_),
    op(tok)
{
}

void StmtJump::Print(ostream& o, int offset)
{
    PrintSpaces(o, offset) << op.GetName() << '\n';
}

void StmtJump::Print(ostream& o, int offset) const
{
    PrintSpaces(o, offset) << op.GetName() << '\n';
}

void StmtJump::Generate(AsmCode& asm_code)
{
    AsmStrImmediate label = op.GetValue() == TOK_BREAK ? loop->GetBreakLabel() : loop->GetContinueLabel();
    asm_code.AddCmd(ASM_JMP, label, SIZE_NONE);
}

bool StmtJump::IsAffectToVar(SymVar* var)
{
    return false;
}

bool StmtJump::IsDependOnVar(SymVar* var)
{
    return false;
}

bool StmtJump::IsHaveSideEffect()
{
    return true;
}

void StmtJump::GetAllAffectedVars(VarsContainer& res_cont)
{
}

StmtClassName StmtJump::GetClassName() const
{
    return STMT_JUMP;
} 

bool StmtJump::CanBeReplaced()
{
    return false;
}

//---StmtExit---

StmtExit::StmtExit(AsmStrImmediate exit_label):
    label(exit_label)
{
}

void StmtExit::Print(ostream& o, int offset)
{
    PrintSpaces(o, offset) << "exit\n";
}

void StmtExit::Print(ostream& o, int offset) const
{
    PrintSpaces(o, offset) << "exit\n";
}

void StmtExit::Generate(AsmCode& asm_code)
{
    asm_code.AddCmd(ASM_JMP, label, SIZE_NONE);
}

bool StmtExit::IsAffectToVar(SymVar* var)
{
    return false;
}

bool StmtExit::IsDependOnVar(SymVar* var)
{
    return false;
}

bool StmtExit::IsHaveSideEffect()
{
    return true;
}

void StmtExit::GetAllAffectedVars(VarsContainer& res_cont)
{
}

StmtClassName StmtExit::GetClassName() const
{
    return STMT_EXIT;
} 

bool StmtExit::CanBeReplaced()
{
    return false;
}
