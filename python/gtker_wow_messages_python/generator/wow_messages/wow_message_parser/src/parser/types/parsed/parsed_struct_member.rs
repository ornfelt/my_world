use crate::parser::types::definer::Definer;
use crate::parser::types::objects::conversion::get_definer;
use crate::parser::types::parsed::parsed_if_statement::ParsedIfStatement;
use crate::parser::types::parsed::parsed_optional::ParsedOptionalStatement;
use crate::parser::types::parsed::parsed_ty::ParsedType;
use crate::parser::types::tags::MemberTags;
use crate::parser::types::{ContainerValue, ParsedContainerValue};
use crate::{ObjectTags, CONTAINER_SELF_SIZE_FIELD};

#[derive(Debug, Clone, Eq, PartialEq)]
pub(crate) enum ParsedStructMember {
    Definition(Box<ParsedStructMemberDefinition>),
    IfStatement(ParsedIfStatement),
    OptionalStatement(ParsedOptionalStatement),
}

impl ParsedStructMember {
    pub(crate) fn is_manual_size_field(&self) -> bool {
        match self {
            ParsedStructMember::Definition(d) => d.is_manual_size_field(),
            ParsedStructMember::IfStatement(_) | ParsedStructMember::OptionalStatement(_) => false,
        }
    }
}

#[derive(Debug, Clone, Eq, PartialEq)]
pub(crate) struct ParsedStructMemberDefinition {
    pub name: String,
    pub(crate) struct_type: ParsedType,
    pub value: Option<ParsedContainerValue>,
    pub verified_value: Option<ContainerValue>,
    pub used_as_size_in: Option<String>,
    pub used_in_if: Option<bool>,
    pub tags: MemberTags,
}

impl ParsedStructMemberDefinition {
    pub(crate) fn struct_type(&self) -> ParsedType {
        self.struct_type.clone()
    }

    pub(crate) fn set_used_as_size_in(&mut self, var: String) {
        self.used_as_size_in = Some(var);
    }

    pub(crate) fn set_used_in_if(&mut self, used: bool) {
        self.used_in_if = Some(used);
    }

    pub(crate) fn is_manual_size_field(&self) -> bool {
        if let Some(v) = &self.value {
            v.identifier == CONTAINER_SELF_SIZE_FIELD
        } else {
            false
        }
    }

    pub(crate) fn new(
        name: &str,
        struct_type: ParsedType,
        value: Option<ParsedContainerValue>,
        tags: MemberTags,
    ) -> Self {
        Self {
            name: name.to_string(),
            struct_type,
            value,
            verified_value: None,
            used_as_size_in: None,
            used_in_if: None,
            tags,
        }
    }

    pub(crate) fn name(&self) -> &str {
        &self.name
    }

    pub(crate) fn ty(&self) -> &ParsedType {
        &self.struct_type
    }

    pub(crate) fn value(&self) -> &Option<ParsedContainerValue> {
        &self.value
    }

    pub(crate) fn set_verified_value(&mut self, definers: &[Definer], tags: &ObjectTags) {
        match &self.value() {
            None => {}
            Some(v) => {
                let parsed_val = crate::parser::utility::parse_value(v.identifier());
                if let Some(int_val) = parsed_val {
                    self.verified_value =
                        Some(ContainerValue::new(int_val, v.identifier().to_string()))
                } else {
                    let value = if v.identifier() != CONTAINER_SELF_SIZE_FIELD {
                        get_definer(definers, &self.ty().str(), tags)
                            .unwrap()
                            .get_field_with_name(v.identifier())
                            .unwrap()
                            .value()
                            .int()
                    } else {
                        0
                    };
                    self.verified_value =
                        Some(ContainerValue::new(value, v.identifier().to_string()));
                }
            }
        }
    }
}
