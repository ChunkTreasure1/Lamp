#include "lppch.h"
#include "Node.h"

namespace Lamp
{
    void Node::ActivateOutput(Attribute& attr)
    {
        if (attr.type == AttributeType::Output && attr.pLink)
        {
            attr.pLink->input->func();
        }
    }
}