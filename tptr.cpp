
#include <Sema/TreeTransform.h>

// from SemaTemplateInstantiate.cpp
namespace clvm {
    
/// \brief Retrieve the depth and index of a parameter pack.
static std::pair<unsigned, unsigned> 
getDepthAndIndex(NamedDecl *ND) {
  if (TemplateTypeParmDecl *TTP = dyn_cast<TemplateTypeParmDecl>(ND))
    return std::make_pair(TTP->getDepth(), TTP->getIndex());
  
  if (NonTypeTemplateParmDecl *NTTP = dyn_cast<NonTypeTemplateParmDecl>(ND))
    return std::make_pair(NTTP->getDepth(), NTTP->getIndex());
  
  TemplateTemplateParmDecl *TTP = cast<TemplateTemplateParmDecl>(ND);
  return std::make_pair(TTP->getDepth(), TTP->getIndex());
}


class TemplateInstantiator : public TreeTransform<TemplateInstantiator> {
    const MultiLevelTemplateArgumentList &TemplateArgs;
    SourceLocation Loc;
    DeclarationName Entity;

  public:
    typedef TreeTransform<TemplateInstantiator> inherited;

    TemplateInstantiator(Sema &SemaRef,
                         const MultiLevelTemplateArgumentList &TemplateArgs,
                         SourceLocation Loc,
                         DeclarationName Entity)
      : inherited(SemaRef), TemplateArgs(TemplateArgs), Loc(Loc),
        Entity(Entity) { }

    /// \brief Determine whether the given type \p T has already been
    /// transformed.
    ///
    /// For the purposes of template instantiation, a type has already been
    /// transformed if it is NULL or if it is not dependent.
    bool AlreadyTransformed(QualType T);

    /// \brief Returns the location of the entity being instantiated, if known.
    SourceLocation getBaseLocation() { return Loc; }

    /// \brief Returns the name of the entity being instantiated, if any.
    DeclarationName getBaseEntity() { return Entity; }

    /// \brief Sets the "base" location and entity when that
    /// information is known based on another transformation.
    void setBase(SourceLocation Loc, DeclarationName Entity) {
      this->Loc = Loc;
      this->Entity = Entity;
    }

    bool TryExpandParameterPacks(SourceLocation EllipsisLoc,
                                 SourceRange PatternRange,
                                 ArrayRef<UnexpandedParameterPack> Unexpanded,
                                 bool &ShouldExpand, bool &RetainExpansion,
                                 Optional<unsigned> &NumExpansions) {
      return getSema().CheckParameterPacksForExpansion(EllipsisLoc, 
                                                       PatternRange, Unexpanded,
                                                       TemplateArgs, 
                                                       ShouldExpand,
                                                       RetainExpansion,
                                                       NumExpansions);
    }

    void ExpandingFunctionParameterPack(ParmVarDecl *Pack) { 
      SemaRef.CurrentInstantiationScope->MakeInstantiatedLocalArgPack(Pack);
    }
    
    TemplateArgument ForgetPartiallySubstitutedPack() {
      TemplateArgument Result;
      if (NamedDecl *PartialPack
            = SemaRef.CurrentInstantiationScope->getPartiallySubstitutedPack()){
        MultiLevelTemplateArgumentList &TemplateArgs
          = const_cast<MultiLevelTemplateArgumentList &>(this->TemplateArgs);
        unsigned Depth, Index;
        std::tie(Depth, Index) = getDepthAndIndex(PartialPack);
        if (TemplateArgs.hasTemplateArgument(Depth, Index)) {
          Result = TemplateArgs(Depth, Index);
          TemplateArgs.setArgument(Depth, Index, TemplateArgument());
        }
      }
      
      return Result;
    }
    
    void RememberPartiallySubstitutedPack(TemplateArgument Arg) {
      if (Arg.isNull())
        return;
      
      if (NamedDecl *PartialPack
            = SemaRef.CurrentInstantiationScope->getPartiallySubstitutedPack()){
        MultiLevelTemplateArgumentList &TemplateArgs
        = const_cast<MultiLevelTemplateArgumentList &>(this->TemplateArgs);
        unsigned Depth, Index;
        std::tie(Depth, Index) = getDepthAndIndex(PartialPack);
        TemplateArgs.setArgument(Depth, Index, Arg);
      }
    }

    /// \brief Transform the given declaration by instantiating a reference to
    /// this declaration.
    Decl *TransformDecl(SourceLocation Loc, Decl *D);

    void transformAttrs(Decl *Old, Decl *New) { 
      SemaRef.InstantiateAttrs(TemplateArgs, Old, New);
    }

    void transformedLocalDecl(Decl *Old, Decl *New) {
      SemaRef.CurrentInstantiationScope->InstantiatedLocal(Old, New);
    }
    
    /// \brief Transform the definition of the given declaration by
    /// instantiating it.
    Decl *TransformDefinition(SourceLocation Loc, Decl *D);

    /// \brief Transform the first qualifier within a scope by instantiating the
    /// declaration.
    NamedDecl *TransformFirstQualifierInScope(NamedDecl *D, SourceLocation Loc);
      
    /// \brief Rebuild the exception declaration and register the declaration
    /// as an instantiated local.
    VarDecl *RebuildExceptionDecl(VarDecl *ExceptionDecl, 
                                  TypeSourceInfo *Declarator,
                                  SourceLocation StartLoc,
                                  SourceLocation NameLoc,
                                  IdentifierInfo *Name);

    /// \brief Rebuild the Objective-C exception declaration and register the 
    /// declaration as an instantiated local.
    VarDecl *RebuildObjCExceptionDecl(VarDecl *ExceptionDecl, 
                                      TypeSourceInfo *TSInfo, QualType T);
      
    /// \brief Check for tag mismatches when instantiating an
    /// elaborated type.
    QualType RebuildElaboratedType(SourceLocation KeywordLoc,
                                   ElaboratedTypeKeyword Keyword,
                                   NestedNameSpecifierLoc QualifierLoc,
                                   QualType T);

    TemplateName
    TransformTemplateName(CXXScopeSpec &SS, TemplateName Name,
                          SourceLocation NameLoc,
                          QualType ObjectType = QualType(),
                          NamedDecl *FirstQualifierInScope = nullptr);

    ExprResult TransformPredefinedExpr(PredefinedExpr *E);
    ExprResult TransformDeclRefExpr(DeclRefExpr *E);
    ExprResult TransformCXXDefaultArgExpr(CXXDefaultArgExpr *E);

    ExprResult TransformTemplateParmRefExpr(DeclRefExpr *E,
                                            NonTypeTemplateParmDecl *D);
    ExprResult TransformSubstNonTypeTemplateParmPackExpr(
                                           SubstNonTypeTemplateParmPackExpr *E);

    /// \brief Rebuild a DeclRefExpr for a ParmVarDecl reference.
    ExprResult RebuildParmVarDeclRefExpr(ParmVarDecl *PD, SourceLocation Loc);

    /// \brief Transform a reference to a function parameter pack.
    ExprResult TransformFunctionParmPackRefExpr(DeclRefExpr *E,
                                                ParmVarDecl *PD);

    /// \brief Transform a FunctionParmPackExpr which was built when we couldn't
    /// expand a function parameter pack reference which refers to an expanded
    /// pack.
    ExprResult TransformFunctionParmPackExpr(FunctionParmPackExpr *E);

    QualType TransformFunctionProtoType(TypeLocBuilder &TLB,
                                        FunctionProtoTypeLoc TL);
    QualType TransformFunctionProtoType(TypeLocBuilder &TLB,
                                        FunctionProtoTypeLoc TL,
                                        CXXRecordDecl *ThisContext,
                                        unsigned ThisTypeQuals);

    ParmVarDecl *TransformFunctionTypeParam(ParmVarDecl *OldParm,
                                            int indexAdjustment,
                                            Optional<unsigned> NumExpansions,
                                            bool ExpectParameterPack);

    /// \brief Transforms a template type parameter type by performing
    /// substitution of the corresponding template type argument.
    QualType TransformTemplateTypeParmType(TypeLocBuilder &TLB,
                                           TemplateTypeParmTypeLoc TL);

    /// \brief Transforms an already-substituted template type parameter pack
    /// into either itself (if we aren't substituting into its pack expansion)
    /// or the appropriate substituted argument.
    QualType TransformSubstTemplateTypeParmPackType(TypeLocBuilder &TLB,
                                           SubstTemplateTypeParmPackTypeLoc TL);

    ExprResult TransformCallExpr(CallExpr *CE) {
      getSema().CallsUndergoingInstantiation.push_back(CE);
      ExprResult Result =
          TreeTransform<TemplateInstantiator>::TransformCallExpr(CE);
      getSema().CallsUndergoingInstantiation.pop_back();
      return Result;
    }

    ExprResult TransformLambdaExpr(LambdaExpr *E) {
      LocalInstantiationScope Scope(SemaRef, /*CombineWithOuterScope=*/true);
      return TreeTransform<TemplateInstantiator>::TransformLambdaExpr(E);
    }

    ExprResult TransformLambdaScope(LambdaExpr *E,
        CXXMethodDecl *NewCallOperator, 
        ArrayRef<InitCaptureInfoTy> InitCaptureExprsAndTypes) {
      CXXMethodDecl *const OldCallOperator = E->getCallOperator();   
      // In the generic lambda case, we set the NewTemplate to be considered
      // an "instantiation" of the OldTemplate.
      if (FunctionTemplateDecl *const NewCallOperatorTemplate = 
            NewCallOperator->getDescribedFunctionTemplate()) {
        
        FunctionTemplateDecl *const OldCallOperatorTemplate = 
                              OldCallOperator->getDescribedFunctionTemplate();
        NewCallOperatorTemplate->setInstantiatedFromMemberTemplate(
                                                     OldCallOperatorTemplate);
      } else 
        // For a non-generic lambda we set the NewCallOperator to 
        // be an instantiation of the OldCallOperator.
        NewCallOperator->setInstantiationOfMemberFunction(OldCallOperator,
                                                    TSK_ImplicitInstantiation);
      
      return inherited::TransformLambdaScope(E, NewCallOperator, 
          InitCaptureExprsAndTypes);
    }
    TemplateParameterList *TransformTemplateParameterList(
                              TemplateParameterList *OrigTPL)  {
      if (!OrigTPL || !OrigTPL->size()) return OrigTPL;
         
      DeclContext *Owner = OrigTPL->getParam(0)->getDeclContext();
      TemplateDeclInstantiator  DeclInstantiator(getSema(), 
                        /* DeclContext *Owner */ Owner, TemplateArgs);
      return DeclInstantiator.SubstTemplateParams(OrigTPL); 
    }
  private:
    ExprResult transformNonTypeTemplateParmRef(NonTypeTemplateParmDecl *parm,
                                               SourceLocation loc,
                                               TemplateArgument arg);
  };


bool TemplateInstantiator::AlreadyTransformed(QualType T) {
  if (T.isNull())
    return true;
  
  if (T->isInstantiationDependentType() || T->isVariablyModifiedType())
    return false;
  
  getSema().MarkDeclarationsReferencedInType(Loc, T);
  return true;
}

static TemplateArgument
getPackSubstitutedTemplateArgument(Sema &S, TemplateArgument Arg) {
  assert(S.ArgumentPackSubstitutionIndex >= 0);        
  assert(S.ArgumentPackSubstitutionIndex < (int)Arg.pack_size());
  Arg = Arg.pack_begin()[S.ArgumentPackSubstitutionIndex];
  if (Arg.isPackExpansion())
    Arg = Arg.getPackExpansionPattern();
  return Arg;
}

Decl *TemplateInstantiator::TransformDecl(SourceLocation Loc, Decl *D) {
  if (!D)
    return nullptr;

  if (TemplateTemplateParmDecl *TTP = dyn_cast<TemplateTemplateParmDecl>(D)) {
    if (TTP->getDepth() < TemplateArgs.getNumLevels()) {
      // If the corresponding template argument is NULL or non-existent, it's
      // because we are performing instantiation from explicitly-specified
      // template arguments in a function template, but there were some
      // arguments left unspecified.
      if (!TemplateArgs.hasTemplateArgument(TTP->getDepth(),
                                            TTP->getPosition()))
        return D;

      TemplateArgument Arg = TemplateArgs(TTP->getDepth(), TTP->getPosition());
      
      if (TTP->isParameterPack()) {
        assert(Arg.getKind() == TemplateArgument::Pack && 
               "Missing argument pack");
        Arg = getPackSubstitutedTemplateArgument(getSema(), Arg);
      }

      TemplateName Template = Arg.getAsTemplate();
      assert(!Template.isNull() && Template.getAsTemplateDecl() &&
             "Wrong kind of template template argument");
      return Template.getAsTemplateDecl();
    }

    // Fall through to find the instantiated declaration for this template
    // template parameter.
  }

  return SemaRef.FindInstantiatedDecl(Loc, cast<NamedDecl>(D), TemplateArgs);
}

Decl *TemplateInstantiator::TransformDefinition(SourceLocation Loc, Decl *D) {
  Decl *Inst = getSema().SubstDecl(D, getSema().CurContext, TemplateArgs);
  if (!Inst)
    return nullptr;

  getSema().CurrentInstantiationScope->InstantiatedLocal(D, Inst);
  return Inst;
}

NamedDecl *
TemplateInstantiator::TransformFirstQualifierInScope(NamedDecl *D, 
                                                     SourceLocation Loc) {
  // If the first part of the nested-name-specifier was a template type 
  // parameter, instantiate that type parameter down to a tag type.
  if (TemplateTypeParmDecl *TTPD = dyn_cast_or_null<TemplateTypeParmDecl>(D)) {
    const TemplateTypeParmType *TTP 
      = cast<TemplateTypeParmType>(getSema().Context.getTypeDeclType(TTPD));
    
    if (TTP->getDepth() < TemplateArgs.getNumLevels()) {
      // FIXME: This needs testing w/ member access expressions.
      TemplateArgument Arg = TemplateArgs(TTP->getDepth(), TTP->getIndex());
      
      if (TTP->isParameterPack()) {
        assert(Arg.getKind() == TemplateArgument::Pack && 
               "Missing argument pack");
        
        if (getSema().ArgumentPackSubstitutionIndex == -1)
          return nullptr;

        Arg = getPackSubstitutedTemplateArgument(getSema(), Arg);
      }

      QualType T = Arg.getAsType();
      if (T.isNull())
        return cast_or_null<NamedDecl>(TransformDecl(Loc, D));
      
      if (const TagType *Tag = T->getAs<TagType>())
        return Tag->getDecl();
      
      // The resulting type is not a tag; complain.
      getSema().Diag(Loc, diag::err_nested_name_spec_non_tag) << T;
      return nullptr;
    }
  }
  
  return cast_or_null<NamedDecl>(TransformDecl(Loc, D));
}

VarDecl *
TemplateInstantiator::RebuildExceptionDecl(VarDecl *ExceptionDecl,
                                           TypeSourceInfo *Declarator,
                                           SourceLocation StartLoc,
                                           SourceLocation NameLoc,
                                           IdentifierInfo *Name) {
  VarDecl *Var = inherited::RebuildExceptionDecl(ExceptionDecl, Declarator,
                                                 StartLoc, NameLoc, Name);
  if (Var)
    getSema().CurrentInstantiationScope->InstantiatedLocal(ExceptionDecl, Var);
  return Var;
}

VarDecl *TemplateInstantiator::RebuildObjCExceptionDecl(VarDecl *ExceptionDecl, 
                                                        TypeSourceInfo *TSInfo, 
                                                        QualType T) {
  VarDecl *Var = inherited::RebuildObjCExceptionDecl(ExceptionDecl, TSInfo, T);
  if (Var)
    getSema().CurrentInstantiationScope->InstantiatedLocal(ExceptionDecl, Var);
  return Var;
}

QualType
TemplateInstantiator::RebuildElaboratedType(SourceLocation KeywordLoc,
                                            ElaboratedTypeKeyword Keyword,
                                            NestedNameSpecifierLoc QualifierLoc,
                                            QualType T) {
  if (const TagType *TT = T->getAs<TagType>()) {
    TagDecl* TD = TT->getDecl();

    SourceLocation TagLocation = KeywordLoc;

    IdentifierInfo *Id = TD->getIdentifier();

    // TODO: should we even warn on struct/class mismatches for this?  Seems
    // like it's likely to produce a lot of spurious errors.
    if (Id && Keyword != ETK_None && Keyword != ETK_Typename) {
      TagTypeKind Kind = TypeWithKeyword::getTagTypeKindForKeyword(Keyword);
      if (!SemaRef.isAcceptableTagRedeclaration(TD, Kind, /*isDefinition*/false,
                                                TagLocation, *Id)) {
        SemaRef.Diag(TagLocation, diag::err_use_with_wrong_tag)
          << Id
          << FixItHint::CreateReplacement(SourceRange(TagLocation),
                                          TD->getKindName());
        SemaRef.Diag(TD->getLocation(), diag::note_previous_use);
      }
    }
  }

  return TreeTransform<TemplateInstantiator>::RebuildElaboratedType(KeywordLoc,
                                                                    Keyword,
                                                                  QualifierLoc,
                                                                    T);
}

TemplateName TemplateInstantiator::TransformTemplateName(CXXScopeSpec &SS,
                                                         TemplateName Name,
                                                         SourceLocation NameLoc,                                     
                                                         QualType ObjectType,
                                             NamedDecl *FirstQualifierInScope) {
  if (TemplateTemplateParmDecl *TTP
       = dyn_cast_or_null<TemplateTemplateParmDecl>(Name.getAsTemplateDecl())) {
    if (TTP->getDepth() < TemplateArgs.getNumLevels()) {
      // If the corresponding template argument is NULL or non-existent, it's
      // because we are performing instantiation from explicitly-specified
      // template arguments in a function template, but there were some
      // arguments left unspecified.
      if (!TemplateArgs.hasTemplateArgument(TTP->getDepth(),
                                            TTP->getPosition()))
        return Name;
      
      TemplateArgument Arg = TemplateArgs(TTP->getDepth(), TTP->getPosition());
      
      if (TTP->isParameterPack()) {
        assert(Arg.getKind() == TemplateArgument::Pack && 
               "Missing argument pack");
        
        if (getSema().ArgumentPackSubstitutionIndex == -1) {
          // We have the template argument pack to substitute, but we're not
          // actually expanding the enclosing pack expansion yet. So, just
          // keep the entire argument pack.
          return getSema().Context.getSubstTemplateTemplateParmPack(TTP, Arg);
        }

        Arg = getPackSubstitutedTemplateArgument(getSema(), Arg);
      }
      
      TemplateName Template = Arg.getAsTemplate();
      assert(!Template.isNull() && "Null template template argument");

      // We don't ever want to substitute for a qualified template name, since
      // the qualifier is handled separately. So, look through the qualified
      // template name to its underlying declaration.
      if (QualifiedTemplateName *QTN = Template.getAsQualifiedTemplateName())
        Template = TemplateName(QTN->getTemplateDecl());

      Template = getSema().Context.getSubstTemplateTemplateParm(TTP, Template);
      return Template;
    }
  }
  
  if (SubstTemplateTemplateParmPackStorage *SubstPack
      = Name.getAsSubstTemplateTemplateParmPack()) {
    if (getSema().ArgumentPackSubstitutionIndex == -1)
      return Name;
    
    TemplateArgument Arg = SubstPack->getArgumentPack();
    Arg = getPackSubstitutedTemplateArgument(getSema(), Arg);
    return Arg.getAsTemplate();
  }
  
  return inherited::TransformTemplateName(SS, Name, NameLoc, ObjectType, 
                                          FirstQualifierInScope);  
}

ExprResult 
TemplateInstantiator::TransformPredefinedExpr(PredefinedExpr *E) {
  if (!E->isTypeDependent())
    return E;

  return getSema().BuildPredefinedExpr(E->getLocation(), E->getIdentType());
}

ExprResult
TemplateInstantiator::TransformTemplateParmRefExpr(DeclRefExpr *E,
                                               NonTypeTemplateParmDecl *NTTP) {
  // If the corresponding template argument is NULL or non-existent, it's
  // because we are performing instantiation from explicitly-specified
  // template arguments in a function template, but there were some
  // arguments left unspecified.
  if (!TemplateArgs.hasTemplateArgument(NTTP->getDepth(),
                                        NTTP->getPosition()))
    return E;

  TemplateArgument Arg = TemplateArgs(NTTP->getDepth(), NTTP->getPosition());
  if (NTTP->isParameterPack()) {
    assert(Arg.getKind() == TemplateArgument::Pack && 
           "Missing argument pack");
    
    if (getSema().ArgumentPackSubstitutionIndex == -1) {
      // We have an argument pack, but we can't select a particular argument
      // out of it yet. Therefore, we'll build an expression to hold on to that
      // argument pack.
      QualType TargetType = SemaRef.SubstType(NTTP->getType(), TemplateArgs,
                                              E->getLocation(), 
                                              NTTP->getDeclName());
      if (TargetType.isNull())
        return ExprError();
      
      return new (SemaRef.Context) SubstNonTypeTemplateParmPackExpr(TargetType,
                                                                    NTTP, 
                                                              E->getLocation(),
                                                                    Arg);
    }
    
    Arg = getPackSubstitutedTemplateArgument(getSema(), Arg);
  }

  return transformNonTypeTemplateParmRef(NTTP, E->getLocation(), Arg);
}

ExprResult TemplateInstantiator::transformNonTypeTemplateParmRef(
                                                 NonTypeTemplateParmDecl *parm,
                                                 SourceLocation loc,
                                                 TemplateArgument arg) {
  ExprResult result;
  QualType type;

  // The template argument itself might be an expression, in which
  // case we just return that expression.
  if (arg.getKind() == TemplateArgument::Expression) {
    Expr *argExpr = arg.getAsExpr();
    result = argExpr;
    type = argExpr->getType();

  } else if (arg.getKind() == TemplateArgument::Declaration ||
             arg.getKind() == TemplateArgument::NullPtr) {
    ValueDecl *VD;
    if (arg.getKind() == TemplateArgument::Declaration) {
      VD = cast<ValueDecl>(arg.getAsDecl());

      // Find the instantiation of the template argument.  This is
      // required for nested templates.
      VD = cast_or_null<ValueDecl>(
             getSema().FindInstantiatedDecl(loc, VD, TemplateArgs));
      if (!VD)
        return ExprError();
    } else {
      // Propagate NULL template argument.
      VD = nullptr;
    }
    
    // Derive the type we want the substituted decl to have.  This had
    // better be non-dependent, or these checks will have serious problems.
    if (parm->isExpandedParameterPack()) {
      type = parm->getExpansionType(SemaRef.ArgumentPackSubstitutionIndex);
    } else if (parm->isParameterPack() && 
               isa<PackExpansionType>(parm->getType())) {
      type = SemaRef.SubstType(
                        cast<PackExpansionType>(parm->getType())->getPattern(),
                                     TemplateArgs, loc, parm->getDeclName());
    } else {
      type = SemaRef.SubstType(parm->getType(), TemplateArgs, 
                               loc, parm->getDeclName());
    }
    assert(!type.isNull() && "type substitution failed for param type");
    assert(!type->isDependentType() && "param type still dependent");
    result = SemaRef.BuildExpressionFromDeclTemplateArgument(arg, type, loc);

    if (!result.isInvalid()) type = result.get()->getType();
  } else {
    result = SemaRef.BuildExpressionFromIntegralTemplateArgument(arg, loc);

    // Note that this type can be different from the type of 'result',
    // e.g. if it's an enum type.
    type = arg.getIntegralType();
  }
  if (result.isInvalid()) return ExprError();

  Expr *resultExpr = result.get();
  return new (SemaRef.Context) SubstNonTypeTemplateParmExpr(
      type, resultExpr->getValueKind(), loc, parm, resultExpr);
}
                                                   
ExprResult 
TemplateInstantiator::TransformSubstNonTypeTemplateParmPackExpr(
                                          SubstNonTypeTemplateParmPackExpr *E) {
  if (getSema().ArgumentPackSubstitutionIndex == -1) {
    // We aren't expanding the parameter pack, so just return ourselves.
    return E;
  }

  TemplateArgument Arg = E->getArgumentPack();
  Arg = getPackSubstitutedTemplateArgument(getSema(), Arg);
  return transformNonTypeTemplateParmRef(E->getParameterPack(),
                                         E->getParameterPackLocation(),
                                         Arg);
}

ExprResult
TemplateInstantiator::RebuildParmVarDeclRefExpr(ParmVarDecl *PD,
                                                SourceLocation Loc) {
  DeclarationNameInfo NameInfo(PD->getDeclName(), Loc);
  return getSema().BuildDeclarationNameExpr(CXXScopeSpec(), NameInfo, PD);
}

ExprResult
TemplateInstantiator::TransformFunctionParmPackExpr(FunctionParmPackExpr *E) {
  if (getSema().ArgumentPackSubstitutionIndex != -1) {
    // We can expand this parameter pack now.
    ParmVarDecl *D = E->getExpansion(getSema().ArgumentPackSubstitutionIndex);
    ValueDecl *VD = cast_or_null<ValueDecl>(TransformDecl(E->getExprLoc(), D));
    if (!VD)
      return ExprError();
    return RebuildParmVarDeclRefExpr(cast<ParmVarDecl>(VD), E->getExprLoc());
  }

  QualType T = TransformType(E->getType());
  if (T.isNull())
    return ExprError();

  // Transform each of the parameter expansions into the corresponding
  // parameters in the instantiation of the function decl.
  SmallVector<Decl *, 8> Parms;
  Parms.reserve(E->getNumExpansions());
  for (FunctionParmPackExpr::iterator I = E->begin(), End = E->end();
       I != End; ++I) {
    ParmVarDecl *D =
        cast_or_null<ParmVarDecl>(TransformDecl(E->getExprLoc(), *I));
    if (!D)
      return ExprError();
    Parms.push_back(D);
  }

  return FunctionParmPackExpr::Create(getSema().Context, T,
                                      E->getParameterPack(),
                                      E->getParameterPackLocation(), Parms);
}

ExprResult
TemplateInstantiator::TransformFunctionParmPackRefExpr(DeclRefExpr *E,
                                                       ParmVarDecl *PD) {
  typedef LocalInstantiationScope::DeclArgumentPack DeclArgumentPack;
  llvm::PointerUnion<Decl *, DeclArgumentPack *> *Found
    = getSema().CurrentInstantiationScope->findInstantiationOf(PD);
  assert(Found && "no instantiation for parameter pack");

  Decl *TransformedDecl;
  if (DeclArgumentPack *Pack = Found->dyn_cast<DeclArgumentPack *>()) {
    // If this is a reference to a function parameter pack which we can substitute
    // but can't yet expand, build a FunctionParmPackExpr for it.
    if (getSema().ArgumentPackSubstitutionIndex == -1) {
      QualType T = TransformType(E->getType());
      if (T.isNull())
        return ExprError();
      return FunctionParmPackExpr::Create(getSema().Context, T, PD,
                                          E->getExprLoc(), *Pack);
    }

    TransformedDecl = (*Pack)[getSema().ArgumentPackSubstitutionIndex];
  } else {
    TransformedDecl = Found->get<Decl*>();
  }

  // We have either an unexpanded pack or a specific expansion.
  return RebuildParmVarDeclRefExpr(cast<ParmVarDecl>(TransformedDecl),
                                   E->getExprLoc());
}

ExprResult
TemplateInstantiator::TransformDeclRefExpr(DeclRefExpr *E) {
  NamedDecl *D = E->getDecl();

  // Handle references to non-type template parameters and non-type template
  // parameter packs.
  if (NonTypeTemplateParmDecl *NTTP = dyn_cast<NonTypeTemplateParmDecl>(D)) {
    if (NTTP->getDepth() < TemplateArgs.getNumLevels())
      return TransformTemplateParmRefExpr(E, NTTP);
    
    // We have a non-type template parameter that isn't fully substituted;
    // FindInstantiatedDecl will find it in the local instantiation scope.
  }

  // Handle references to function parameter packs.
  if (ParmVarDecl *PD = dyn_cast<ParmVarDecl>(D))
    if (PD->isParameterPack())
      return TransformFunctionParmPackRefExpr(E, PD);

  return TreeTransform<TemplateInstantiator>::TransformDeclRefExpr(E);
}

ExprResult TemplateInstantiator::TransformCXXDefaultArgExpr(
    CXXDefaultArgExpr *E) {
  assert(!cast<FunctionDecl>(E->getParam()->getDeclContext())->
             getDescribedFunctionTemplate() &&
         "Default arg expressions are never formed in dependent cases.");
  return SemaRef.BuildCXXDefaultArgExpr(E->getUsedLocation(),
                           cast<FunctionDecl>(E->getParam()->getDeclContext()), 
                                        E->getParam());
}

QualType TemplateInstantiator::TransformFunctionProtoType(TypeLocBuilder &TLB,
                                                      FunctionProtoTypeLoc TL) {
  // We need a local instantiation scope for this function prototype.
  LocalInstantiationScope Scope(SemaRef, /*CombineWithOuterScope=*/true);
  return inherited::TransformFunctionProtoType(TLB, TL);
}

QualType TemplateInstantiator::TransformFunctionProtoType(TypeLocBuilder &TLB,
                                 FunctionProtoTypeLoc TL,
                                 CXXRecordDecl *ThisContext,
                                 unsigned ThisTypeQuals) {
  // We need a local instantiation scope for this function prototype.
  LocalInstantiationScope Scope(SemaRef, /*CombineWithOuterScope=*/true);
  return inherited::TransformFunctionProtoType(TLB, TL, ThisContext, 
                                               ThisTypeQuals);  
}

ParmVarDecl *
TemplateInstantiator::TransformFunctionTypeParam(ParmVarDecl *OldParm,
                                                 int indexAdjustment,
                                               Optional<unsigned> NumExpansions,
                                                 bool ExpectParameterPack) {
  return SemaRef.SubstParmVarDecl(OldParm, TemplateArgs, indexAdjustment,
                                  NumExpansions, ExpectParameterPack);
}

QualType
TemplateInstantiator::TransformTemplateTypeParmType(TypeLocBuilder &TLB,
                                                TemplateTypeParmTypeLoc TL) {
  const TemplateTypeParmType *T = TL.getTypePtr();
  if (T->getDepth() < TemplateArgs.getNumLevels()) {
    // Replace the template type parameter with its corresponding
    // template argument.

    // If the corresponding template argument is NULL or doesn't exist, it's
    // because we are performing instantiation from explicitly-specified
    // template arguments in a function template class, but there were some
    // arguments left unspecified.
    if (!TemplateArgs.hasTemplateArgument(T->getDepth(), T->getIndex())) {
      TemplateTypeParmTypeLoc NewTL
        = TLB.push<TemplateTypeParmTypeLoc>(TL.getType());
      NewTL.setNameLoc(TL.getNameLoc());
      return TL.getType();
    }

    TemplateArgument Arg = TemplateArgs(T->getDepth(), T->getIndex());
    
    if (T->isParameterPack()) {
      assert(Arg.getKind() == TemplateArgument::Pack && 
             "Missing argument pack");
      
      if (getSema().ArgumentPackSubstitutionIndex == -1) {
        // We have the template argument pack, but we're not expanding the
        // enclosing pack expansion yet. Just save the template argument
        // pack for later substitution.
        QualType Result
          = getSema().Context.getSubstTemplateTypeParmPackType(T, Arg);
        SubstTemplateTypeParmPackTypeLoc NewTL
          = TLB.push<SubstTemplateTypeParmPackTypeLoc>(Result);
        NewTL.setNameLoc(TL.getNameLoc());
        return Result;
      }
      
      Arg = getPackSubstitutedTemplateArgument(getSema(), Arg);
    }
    
    assert(Arg.getKind() == TemplateArgument::Type &&
           "Template argument kind mismatch");

    QualType Replacement = Arg.getAsType();

    // TODO: only do this uniquing once, at the start of instantiation.
    QualType Result
      = getSema().Context.getSubstTemplateTypeParmType(T, Replacement);
    SubstTemplateTypeParmTypeLoc NewTL
      = TLB.push<SubstTemplateTypeParmTypeLoc>(Result);
    NewTL.setNameLoc(TL.getNameLoc());
    return Result;
  }

  // The template type parameter comes from an inner template (e.g.,
  // the template parameter list of a member template inside the
  // template we are instantiating). Create a new template type
  // parameter with the template "level" reduced by one.
  TemplateTypeParmDecl *NewTTPDecl = nullptr;
  if (TemplateTypeParmDecl *OldTTPDecl = T->getDecl())
    NewTTPDecl = cast_or_null<TemplateTypeParmDecl>(
                                  TransformDecl(TL.getNameLoc(), OldTTPDecl));

  QualType Result
    = getSema().Context.getTemplateTypeParmType(T->getDepth()
                                                 - TemplateArgs.getNumLevels(),
                                                T->getIndex(),
                                                T->isParameterPack(),
                                                NewTTPDecl);
  TemplateTypeParmTypeLoc NewTL = TLB.push<TemplateTypeParmTypeLoc>(Result);
  NewTL.setNameLoc(TL.getNameLoc());
  return Result;
}

QualType 
TemplateInstantiator::TransformSubstTemplateTypeParmPackType(
                                                            TypeLocBuilder &TLB,
                                         SubstTemplateTypeParmPackTypeLoc TL) {
  if (getSema().ArgumentPackSubstitutionIndex == -1) {
    // We aren't expanding the parameter pack, so just return ourselves.
    SubstTemplateTypeParmPackTypeLoc NewTL
      = TLB.push<SubstTemplateTypeParmPackTypeLoc>(TL.getType());
    NewTL.setNameLoc(TL.getNameLoc());
    return TL.getType();
  }

  TemplateArgument Arg = TL.getTypePtr()->getArgumentPack();
  Arg = getPackSubstitutedTemplateArgument(getSema(), Arg);
  QualType Result = Arg.getAsType();

  Result = getSema().Context.getSubstTemplateTypeParmType(
                                      TL.getTypePtr()->getReplacedParameter(),
                                                          Result);
  SubstTemplateTypeParmTypeLoc NewTL
    = TLB.push<SubstTemplateTypeParmTypeLoc>(Result);
  NewTL.setNameLoc(TL.getNameLoc());
  return Result;
}

};
