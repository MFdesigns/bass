// ======================================================================== //
// Copyright 2020 Michel Fäh
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ======================================================================== //

#include "asm.hpp"
#include "encoding.hpp"
#include <cstdint>
#include <vector>

/**
 * InstrDefNode move constructor
 * @param instrDefNode Right value reference of InstrDefNode to be moved
 */
InstrDefNode::InstrDefNode(InstrDefNode&& instrDefNode) noexcept {
    Type = instrDefNode.Type;
    Children = std::move(instrDefNode.Children);
    ParamList = instrDefNode.ParamList;
}

/**
 * InstrDefNode constructor
 * @param type Paramter type
 * @param paramList Pointer to encoding information if this is an end node of
 * the instruction signature otherwise pass nullptr
 */
InstrDefNode::InstrDefNode(InstrParamType type, InstrParamList* paramList)
    : Type(type), ParamList(paramList) {}

/**
 * Builds the tree data struct used for instruction resolution
 * @param target Output instruction definitions tree
 */
void buildInstrDefTree(std::vector<InstrDefNode>& target) {
    for (const auto& def : Asm::INSTR_ASM_DEFS) {
        InstrDefNode instr{};
        for (const InstrParamList& paramList : def) {
            // Keeps track of the current parent node where the children will be
            // appended
            InstrDefNode* parent = &instr;
            for (const InstrParamType paramType : paramList.Params) {
                // Check if param type is already a child of the current cursor
                // node. If existingChild is a nullptr no already existing
                // children were found.
                InstrDefNode* existingChild = nullptr;
                for (uint32_t i = 0; i < parent->Children.size(); i++) {
                    InstrParamType type = parent->Children[i].Type;
                    if (type == paramType) {
                        existingChild = &parent->Children[i];
                        break;
                    }
                }

                if (existingChild == nullptr) {
                    // If parent does not already have this type as a child add
                    // it
                    parent->Children.emplace_back(paramType, nullptr);
                    // Set parent to last pushed child
                    parent = &parent->Children[parent->Children.size() - 1];
                } else {
                    parent = existingChild;
                }
            }
            // After adding the last node of the branch add pointer to encoding
            // information
            parent->ParamList = (InstrParamList*)&paramList;
        }
        target.emplace_back(std::move(instr));
    }
}
