// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

#include "core/text/TextProcessor.h"

#include <QString>
#include <QVector>

namespace dictapulse {

class Settings;

/// Composes the user's Profile (dictionary, voice templates, tone, refine
/// behaviors, dev-mode jargon) into the inputs every AI touchpoint consumes:
///
///   • whisperVocabulary()        → Whisper `initial_prompt` (spelling/context bias)
///   • systemPromptAugmentation() → appended to the cleanup-LLM system prompt
///   • replacements()             → deterministic find/replace + template expansion
///
/// This is what lets *any* plugged-in model - rules, local LLM, or remote API -
/// read from the Profile and produce more precise transcriptions and responses.
namespace ProfileContext {

/// Dictionary rules + voice templates as one ordered replacement list.
QVector<TextProcessor::Replacement> replacements(const Settings* s);

/// Space/comma-joined vocabulary (dictionary targets, dev jargon, template
/// cues) for Whisper's initial_prompt. Capped well under the token budget.
QString whisperVocabulary(const Settings* s);

/// Natural-language instruction block describing the user's tone, refine
/// preferences, dev context, dictionary and templates - appended to the
/// cleanup LLM's system prompt. Empty when nothing is active.
QString systemPromptAugmentation(const Settings* s);

} // namespace ProfileContext

} // namespace dictapulse
