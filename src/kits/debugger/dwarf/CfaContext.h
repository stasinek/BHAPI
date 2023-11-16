/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef CFA_CONTEXT_H
#define CFA_CONTEXT_H


#include <ObjectList.h>

#include <CfaRuleSet.h>
#include <Types.h>


class CfaContext {
public:
								CfaContext();
								~CfaContext();

			void				SetLocation(target_addr_t targetLocation,
									target_addr_t initialLocation);

			status_t			Init(uint32_t registerCount);
			status_t			SaveInitialRuleSet();

			status_t			PushRuleSet();
			status_t			PopRuleSet();

			target_addr_t		TargetLocation() const
									{ return fTargetLocation; }

			target_addr_t		Location() const
									{ return fLocation; }
			void				SetLocation(target_addr_t location);

			uint32_t				CodeAlignment() const
									{ return fCodeAlignment; }
			void				SetCodeAlignment(uint32_t alignment);

			int32_t				DataAlignment() const
									{ return fDataAlignment; }
			void				SetDataAlignment(int32_t alignment);

			uint32_t				ReturnAddressRegister() const
									{ return fReturnAddressRegister; }
			void				SetReturnAddressRegister(uint32_t reg);

			CfaCfaRule*			GetCfaCfaRule() const
									{ return fRuleSet->GetCfaCfaRule(); }
			CfaRule*			RegisterRule(uint32_t index) const
									{ return fRuleSet->RegisterRule(index); }

			void				RestoreRegisterRule(uint32_t reg);

private:
			typedef BObjectList<CfaRuleSet> RuleSetList;

private:
			target_addr_t		fTargetLocation;
			target_addr_t		fLocation;
			uint32_t				fCodeAlignment;
			int32_t				fDataAlignment;
			uint32_t				fReturnAddressRegister;
			CfaRuleSet*			fRuleSet;
			CfaRuleSet*			fInitialRuleSet;
			RuleSetList			fRuleSetStack;
};



#endif	// CFA_CONTEXT_H
