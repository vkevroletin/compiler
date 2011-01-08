#ifndef SYNTAX_NODE_BASE
#define SYNTAX_NODE_BASE

#include "scanner.h"
#include "generator.h"
#include <ostream>

class SymType;

extern SymType* top_type_int;
extern SymType* top_type_real;
extern SymType* top_type_untyped;
extern SymType* top_type_str;

class SyntaxNode{
public:
    virtual void Print(std::ostream& o, int offset = 0) const;
    virtual const SymType* GetSymType() const;
    virtual bool IsLValue() const;
    virtual void GenerateLValue(AsmCode& asm_code) const;    
    virtual void GenerateValue(AsmCode& asm_code) const;
    virtual bool IsConst() const;
    virtual Token ComputeConstExpr() const;
    virtual int ComputeIntConstExpr() const;
    virtual float ComputeRealConstExpr() const;
};

#endif
