#include "ResourceState.h"

const Engine::ResourceCapabilities Engine::ResourceCapabilities::PiplineInput = Engine::ResourceCapabilities{ false, false, false };
const Engine::ResourceCapabilities Engine::ResourceCapabilities::Read = Engine::ResourceCapabilities{ true, false, false };
const Engine::ResourceCapabilities Engine::ResourceCapabilities::ReadWrite = Engine::ResourceCapabilities{ true, true, false };
const Engine::ResourceCapabilities Engine::ResourceCapabilities::Transient = Engine::ResourceCapabilities{ false, false, true };

