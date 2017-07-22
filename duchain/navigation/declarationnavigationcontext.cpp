/*************************************************************************************
*  Copyright (C) 2014 by Pavel Petrushkov <onehundredof@gmail.com>                  *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

//most of this code is just a copy of AbstractDeclarationNavigationContext
//because default html behaviour really doesn't work for go
//mostly because it skips type declaration which are not structure types(identified types)
//TODO come up with a better way to do all this

#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/structuretype.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>

//...
#include <language/duchain/types/typealiastype.h>
#include <language/duchain/types/structuretype.h>
#include <language/duchain/classdeclaration.h>
#include <typeinfo>
#include "language/duchain/functiondeclaration.h"
#include "language/duchain/functiondefinition.h"
#include "language/duchain/classfunctiondeclaration.h"
#include "language/duchain/namespacealiasdeclaration.h"
#include "language/duchain/forwarddeclaration.h"
#include "language/duchain/types/enumeratortype.h"
#include "language/duchain/types/enumerationtype.h"
#include "language/duchain/types/functiontype.h"
#include "language/duchain/duchainutils.h"
#include "language/duchain/types/pointertype.h"
#include "language/duchain/types/referencetype.h"
#include "language/duchain/types/typeutils.h"
#include "language/duchain/persistentsymboltable.h"
#include "language/duchain/types/arraytype.h"


#include <QtGui/QTextDocument>
#include <declarations/functiondefinition.h>

#include "navigation/declarationnavigationcontext.h"
#include "declarations/functiondeclaration.h"
#include "types/gofunctiontype.h"
#include "../duchaindebug.h"

using namespace KDevelop;

DeclarationNavigationContext::DeclarationNavigationContext(DeclarationPointer decl,
                                                                  KDevelop::TopDUContextPointer topContext,
                                                                  AbstractNavigationContext* previousContext)
: AbstractDeclarationNavigationContext(decl, topContext, previousContext)
{

}

QString DeclarationNavigationContext::html(bool shorten)
{
  clear();
  AbstractNavigationContext::html(shorten);
  modifyHtml()  += "<html><body><p>" + fontSizePrefix(shorten);

  addExternalHtml(prefix());

  if(!declaration().data()) {
    modifyHtml() += i18n("<br /> lost declaration <br />");
    return currentHtml();
  }
  
  if( previousContext() ) {
    QString link = createLink( previousContext()->name(), previousContext()->name(), NavigationAction(previousContext()) );
    modifyHtml() += navigationHighlight(i18n("Back to %1<br />", link));
  }
  
  QExplicitlySharedDataPointer<IDocumentation> doc;
  
  if( !shorten ) {
    doc = ICore::self()->documentationController()->documentationForDeclaration(declaration().data());

    const AbstractFunctionDeclaration* function = dynamic_cast<const AbstractFunctionDeclaration*>(declaration().data());
    if( function ) {
      htmlFunction();
    } else if( declaration()->isTypeAlias() || declaration()->kind() == Declaration::Instance ) {
      if( declaration()->isTypeAlias() )
        modifyHtml() += importantHighlight("type ");

      if(declaration()->type<EnumeratorType>())
        modifyHtml() += i18n("enumerator ");
      
      if( !declaration()->isTypeAlias())
          modifyHtml() += ' ' + identifierHighlight(declarationName(declaration()).toHtmlEscaped(), declaration()) + " ";

      AbstractType::Ptr useType = declaration()->abstractType();
      if(declaration()->isTypeAlias()) {
        //Do not show the own name as type of typedefs
        if(useType.cast<TypeAliasType>())
          useType = useType.cast<TypeAliasType>()->type();
      } 
      
      eventuallyMakeTypeLinks( useType );
      
      modifyHtml() += "<br>";

    }else{
      if( declaration()->kind() == Declaration::Type && declaration()->abstractType().cast<StructureType>()) {
        htmlClass();
      }
      if ( declaration()->kind() == Declaration::Namespace ) {
        modifyHtml() += i18n("namespace %1 ", identifierHighlight(declaration()->qualifiedIdentifier().toString().toHtmlEscaped(), declaration()));
      }

      if(declaration()->type<EnumerationType>()) {
        EnumerationType::Ptr enumeration = declaration()->type<EnumerationType>();
        modifyHtml() += i18n("enumeration %1 ", identifierHighlight(declaration()->identifier().toString().toHtmlEscaped(), declaration()));
      }

      if(declaration()->isForwardDeclaration()) {
        ForwardDeclaration* forwardDec = static_cast<ForwardDeclaration*>(declaration().data());
        Declaration* resolved = forwardDec->resolve(topContext().data());
        if(resolved) {
          modifyHtml() += i18n("( resolved forward-declaration: ");
          makeLink(resolved->identifier().toString(), KDevelop::DeclarationPointer(resolved), NavigationAction::NavigateDeclaration );
          modifyHtml() += i18n(") ");
        }else{
          modifyHtml() += i18n("(unresolved forward-declaration) ");
          QualifiedIdentifier id = forwardDec->qualifiedIdentifier();
          uint count;
          const IndexedDeclaration* decls;
          PersistentSymbolTable::self().declarations(id, count, decls);
          for(uint a = 0; a < count; ++a) {
            if(decls[a].isValid() && !decls[a].data()->isForwardDeclaration()) {
              modifyHtml() += "<br />";
              makeLink(i18n("possible resolution from"), KDevelop::DeclarationPointer(decls[a].data()), NavigationAction::NavigateDeclaration);
              modifyHtml() += ' ' + decls[a].data()->url().str();
            }
          }
        }
      }
      modifyHtml() += "<br />";
    }
  }else{
    AbstractType::Ptr showType = declaration()->abstractType();
    if(showType && showType.cast<FunctionType>()) {
      showType = showType.cast<FunctionType>()->returnType();
      if(showType)
        modifyHtml() += labelHighlight(i18n("Returns: "));
    }else  if(showType) {
      modifyHtml() += labelHighlight(i18n("Type: "));
    }
    
    if(showType) {
      eventuallyMakeTypeLinks(showType);
      modifyHtml() += " ";
    }
  }
  
  QualifiedIdentifier identifier = declaration()->qualifiedIdentifier();
  if( identifier.count() > 1 ) {
    if( declaration()->context() && declaration()->context()->owner() )
    {
      Declaration* decl = declaration()->context()->owner();

      FunctionDefinition* definition = dynamic_cast<FunctionDefinition*>(decl);
      if(definition && definition->declaration())
        decl = definition->declaration();

      if(decl->abstractType().cast<EnumerationType>())
        modifyHtml() += labelHighlight(i18n("Enum: "));
      else
        modifyHtml() += labelHighlight(i18n("Container: "));

      makeLink( declarationName(DeclarationPointer(decl)), DeclarationPointer(decl), NavigationAction::NavigateDeclaration );
      modifyHtml() += " ";
    } else {
      QualifiedIdentifier parent = identifier;
      parent.pop();
      modifyHtml() += labelHighlight(i18n("Scope: %1 ", typeHighlight(parent.toString().toHtmlEscaped())));
    }
  }
  
  if( shorten && !declaration()->comment().isEmpty() ) {
    QString comment = QString::fromUtf8(declaration()->comment());
    if( comment.length() > 60 ) {
      comment.truncate(60);
      comment += "...";
    }
    comment.replace('\n', " ");
    comment.replace("<br />", " ");
    comment.replace("<br/>", " ");
    modifyHtml() += commentHighlight(comment.toHtmlEscaped()) + "   ";
  }
  

  QString access = stringFromAccess(declaration());
  if( !access.isEmpty() )
    modifyHtml() += labelHighlight(i18n("Access: %1 ", propertyHighlight(access.toHtmlEscaped())));


  ///@todo Enumerations

  QString detailsHtml;
  QStringList details = declarationDetails(declaration());
  if( !details.isEmpty() ) {
    bool first = true;
    foreach( const QString &str, details ) {
      if( !first )
        detailsHtml += ", ";
      first = false;
      detailsHtml += propertyHighlight(str);
    }
  }

  QString kind = declarationKind(declaration());
  if( !kind.isEmpty() ) {
    if( !detailsHtml.isEmpty() )
      modifyHtml() += labelHighlight(i18n("Kind: %1 %2 ", importantHighlight(kind.toHtmlEscaped()), detailsHtml));
    else
      modifyHtml() += labelHighlight(i18n("Kind: %1 ", importantHighlight(kind.toHtmlEscaped())));
  } else if( !detailsHtml.isEmpty() ) {
    modifyHtml() += labelHighlight(i18n("Modifiers: %1 ",  importantHighlight(kind.toHtmlEscaped())));
  }

  modifyHtml() += "<br />";

  if(!shorten)
    htmlAdditionalNavigation();
  
  if( !shorten ) {
    if(dynamic_cast<FunctionDefinition*>(declaration().data()))
      modifyHtml() += labelHighlight(i18n( "Def.: " ));
    else
      modifyHtml() += labelHighlight(i18n( "Decl.: " ));

    makeLink( QString("%1 :%2").arg( QUrl(declaration()->url().str()).fileName() ).arg( declaration()->rangeInCurrentRevision().start().line()+1 ), declaration(), NavigationAction::JumpToSource );
    modifyHtml() += " ";
    //modifyHtml() += "<br />";
    if(!dynamic_cast<FunctionDefinition*>(declaration().data())) {
      if( FunctionDefinition* definition = FunctionDefinition::definition(declaration().data()) ) {
        modifyHtml() += labelHighlight(i18n( " Def.: " ));
        makeLink( QString("%1 :%2").arg( QUrl(definition->url().str()).fileName() ).arg( definition->rangeInCurrentRevision().start().line()+1 ), DeclarationPointer(definition), NavigationAction::JumpToSource );
      }
    }

    if( FunctionDefinition* definition = dynamic_cast<FunctionDefinition*>(declaration().data()) ) {
      if(definition->declaration()) {
        modifyHtml() += labelHighlight(i18n( " Decl.: " ));
        makeLink( QString("%1 :%2").arg( QUrl(definition->declaration()->url().str()).fileName() ).arg( definition->declaration()->rangeInCurrentRevision().start().line()+1 ), DeclarationPointer(definition->declaration()), NavigationAction::JumpToSource );
      }
    }
    
    modifyHtml() += " "; //The action name _must_ stay "show_uses", since that is also used from outside
    makeLink(i18n("Show uses"), "show_uses", NavigationAction(declaration(), NavigationAction::NavigateUses));
  }
  
  if( !shorten && (!declaration()->comment().isEmpty() || doc) ) {
    modifyHtml() += "<br />";
    QString comment = QString::fromUtf8(declaration()->comment());
    if(comment.isEmpty() && doc) {
      comment = doc->description();
      if(!comment.isEmpty()) {
        connect(doc.data(), SIGNAL(descriptionChanged()), this, SIGNAL(contentsChanged()));
        modifyHtml() += "<br />" + commentHighlight(comment);
      }
    } else if(!comment.isEmpty()) {
      comment.replace(QRegExp("<br */>"), "\n"); //do not escape html newlines within the comment
      comment = comment.toHtmlEscaped();
      comment.replace('\n', "<br />"); //Replicate newlines in html
      modifyHtml() += commentHighlight(comment);
      modifyHtml() += "<br />";
    }
  }
  
    if(!shorten && doc) {
      modifyHtml() += "<br />" + i18n("Show documentation for ");
      makeLink( prettyQualifiedIdentifier(declaration()).toString(), declaration(), NavigationAction::ShowDocumentation );
    }
  
  
    //modifyHtml() += "<br />";

  addExternalHtml(suffix());

  modifyHtml() += fontSizeSuffix(shorten) + "</p></body></html>";

  return currentHtml();
}


void DeclarationNavigationContext::htmlFunction()
{
    //KDevelop::AbstractDeclarationNavigationContext::htmlFunction();
    go::GoFunctionDeclaration* function = dynamic_cast<go::GoFunctionDeclaration*>(declaration().data());
    go::GoFunctionDefinition* functionDefinition = dynamic_cast<go::GoFunctionDefinition*>(declaration().data());
    if(!function && functionDefinition)
    {
        Declaration *decl = DUChainUtils::declarationForDefinition(functionDefinition, topContext().data());
        function = dynamic_cast<go::GoFunctionDeclaration*>(decl);
    }
    if(function && !functionDefinition)
    {
        DUChainReadLocker lock;
        auto decls = function->context()->findDeclarations(function->qualifiedIdentifier());
        for(auto decl : decls)
        {
            auto definition = dynamic_cast<go::GoFunctionDefinition*>(decl);
            if(definition)
            {
                functionDefinition = definition;
            }
        }
    }
    if(!function && !functionDefinition)
        AbstractDeclarationNavigationContext::htmlFunction();

    const go::GoFunctionType::Ptr type = declaration()->abstractType().cast<go::GoFunctionType>();
    if( !type ) {
        modifyHtml() += errorHighlight("Invalid type<br />");
        return;
    }

    /*if( !classFunDecl || (!classFunDecl->isConstructor() && !classFunDecl->isDestructor()) ) {
        // only print return type for global functions and non-ctor/dtor methods
        eventuallyMakeTypeLinks( type->returnType() );
    }*/

    modifyHtml() += ' ' + identifierHighlight(prettyIdentifier(declaration()).toString().toHtmlEscaped(), declaration());

    if( type->indexedArgumentsSize() == 0 )
    {
        modifyHtml() += "()";
    } else {
        modifyHtml() += "( ";

        bool first = true;
        //int firstDefaultParam = type->indexedArgumentsSize() - function->defaultParametersSize();
        int currentArgNum = 0;

        QVector<Declaration*> decls;
        if (KDevelop::DUContext* argumentContext = DUChainUtils::getArgumentContext(function)) {
            decls = argumentContext->localDeclarations(topContext().data());
        }
        else if (KDevelop::DUContext* argumentContext = DUChainUtils::getArgumentContext(functionDefinition)) {
            decls = argumentContext->localDeclarations(topContext().data());
        }
        foreach(const AbstractType::Ptr& argType, type->arguments()) {
            if( !first )
                modifyHtml() += ", ";
            first = false;

            if (currentArgNum < decls.size()) {
                modifyHtml() += identifierHighlight(decls[currentArgNum]->identifier().toString().toHtmlEscaped(), declaration()) + " ";
            }

            if(type->modifiers() == go::GoFunctionType::VariadicArgument && currentArgNum == decls.size()-1)
            {
                modifyHtml() += "...";
                if(fastCast<ArrayType*>(argType.constData()))
                {//show only element type in variadic parameter
                    eventuallyMakeTypeLinks(fastCast<ArrayType*>(argType.constData())->elementType());
                }else
                {//this shouldn't happen
                    qCDebug(DUCHAIN) << "Variadic type was not resolved to slice type";
                    eventuallyMakeTypeLinks( argType );
                }
            }else
            {
                eventuallyMakeTypeLinks( argType );
            }

            /*if( currentArgNum >= firstDefaultParam )
                modifyHtml() += " = " + Qt::escape(function->defaultParameters()[ currentArgNum - firstDefaultParam ].str());*/

            ++currentArgNum;
        }

        modifyHtml() += " )";
    }
    //return types
    qCDebug(DUCHAIN) << type->returnArguments().size();
    if(type->returnArguments().size() != 0)
    {
        modifyHtml() += " ";
        int currentArgNum = 0;
        bool first=true;
        QVector<Declaration*> decls;
        /*if (KDevelop::DUContext* argumentContext = DUChainUtils::getArgumentContext(declaration().data())) {
            decls = argumentContext->localDeclarations(topContext().data());
        }*/
        if(DUContext* retContext = function->returnArgsContext())
            decls = retContext->localDeclarations(topContext().data());
        else if(DUContext* retContext = functionDefinition->returnArgsContext())
            decls = retContext->localDeclarations(topContext().data());
        
        if(type->returnArguments().size() == 1)
        {
            if(decls.size() != 0) //show declaration if one exists
                modifyHtml() += identifierHighlight(decls[0]->identifier().toString().toHtmlEscaped(), declaration()) + " ";
            eventuallyMakeTypeLinks(type->returnArguments().front());
            //modifyHtml() += ' ' + nameHighlight(Qt::escape(decls[currentArgNum]->identifier().toString()));
        }
        else
        {
            modifyHtml() += "(";
            foreach(const AbstractType::Ptr& argType, type->returnArguments())
            {
                if( !first )
                    modifyHtml() += ", ";
                first = false;

                //TODO fix parameter names
                if (currentArgNum < decls.size()) {
                    modifyHtml() += identifierHighlight(decls[currentArgNum]->identifier().toString().toHtmlEscaped(), declaration()) + " ";
                }
                eventuallyMakeTypeLinks( argType );
                ++currentArgNum;
            }
            modifyHtml() += ")";
        }
    }
        
    modifyHtml() += "<br />";
}

void DeclarationNavigationContext::eventuallyMakeTypeLinks(AbstractType::Ptr type)
{
    if( !type)
    {
        modifyHtml() += typeHighlight(QString("<no type>").toHtmlEscaped());
        return;
    }
    if(declaration()->isTypeAlias())
    {
        //Go type declaration. manually creating links
        QualifiedIdentifier id = declaration()->qualifiedIdentifier();
        makeLink(id.toString(), DeclarationPointer(declaration()), NavigationAction::NavigateDeclaration );
    }
    else
    {
        KDevelop::AbstractDeclarationNavigationContext::eventuallyMakeTypeLinks(type);
    }
}
