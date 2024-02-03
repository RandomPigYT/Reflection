#include "handleDecl.hpp"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecordLayout.h>

#include "ast.hpp"

void Reflection::handleRecordDecl(clang::RecordDecl *rd,
                                  struct context_t &ctx) {
  if (!rd->isStruct() && !rd->isUnion()) return;

  if (rd->isAnonymousStructOrUnion()) return;

  Reflection::typeInfo_t t;

  t.ID = rd->getID();
  t.fileName.assign(ctx.filename);
  t.name = rd->getNameAsString();
  t.recordType = rd->isStruct() ? RECORD_TYPE_STRUCT : RECORD_TYPE_UNION;

  ctx.typeinfo.emplace_back(t);
}

void Reflection::handleTypedefDecl(clang::TypedefDecl *td,
                                   struct context_t &ctx) {
  clang::QualType q = td->getUnderlyingType();

  clang::RecordDecl *rd = q->getAsRecordDecl();
  if (!rd || (!rd->isStruct() && !rd->isUnion())) return;

  for (uint64_t i = 0; i < ctx.typeinfo.size(); i++) {
    if (ctx.typeinfo[i].fileName != ctx.filename ||
        ctx.typeinfo[i].ID != rd->getID())
      continue;

		printf("%ld\n", i);

    ctx.typeinfo[i].aliases.emplace_back(td->getNameAsString());
  }
}


static int64_t findRecord(int64_t ID, std::string filename, const struct Reflection::context_t &ctx){
	for (uint64_t i = 0; i < ctx.typeinfo.size(); i++){
		if (ctx.typeinfo[i].ID == ID && ctx.typeinfo[i].fileName == filename)
			return i;
	}

	return -1;
}


void Reflection::handleFieldDecl(clang::FieldDecl *fd, struct context_t &ctx) {
  const clang::ASTRecordLayout &layout =
      ctx.context->getASTRecordLayout(fd->getParent());

  Reflection::field_t f;
  f.name = fd->getNameAsString();
  f.offset = layout.getFieldOffset(fd->getFieldIndex());

  clang::QualType fieldType = fd->getType();

  if (clang::RecordDecl *rd = fieldType->getAsRecordDecl()) {
    Reflection::typeSpecifier_t spec;
    spec.type = rd->isStruct() ? Reflection::FIELD_TYPE_STRUCT
                               : Reflection::FIELD_TYPE_UNION;
    spec.info = new Reflection::recordRef_t;

    Reflection::recordRef_t *temp = (Reflection::recordRef_t *)spec.info;
    temp->ID = rd->getID();
    temp->fileName = ctx.filename;

    f.type = spec;
  
  	ctx.typeinfo[findRecord(fd->getParent()->getID(), ctx.filename, ctx)].fields.emplace_back(f);

    return;
  }

	

  // clang::Decl *declForField =
  // clang::dyn_cast<clang::Decl>(fd->getUnderlyingDecl());
}
