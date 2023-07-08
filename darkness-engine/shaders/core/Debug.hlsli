
struct DebugItem
{
    uint itemType;
    uint uvalue;
    float fvalue;
};

AppendStructuredBuffer<DebugItem> debugOutput;

void shaderDebug(bool val)
{
    DebugItem item;
    item.itemType = 1;
    item.uvalue = (uint)val;
    item.fvalue = 0.0f;
    debugOutput.Append(item);
};

void shaderDebug(int val)
{
    DebugItem item;
    item.itemType = 2;
    item.uvalue = (uint)val;
    item.fvalue = 0.0f;
    debugOutput.Append(item);
};

void shaderDebug(uint val)
{
    DebugItem item;
    item.itemType = 3;
    item.uvalue = (uint)val;
    item.fvalue = 0.0f;
    debugOutput.Append(item);
};

void shaderDebug(float val)
{
    DebugItem item;
    item.itemType = 4;
    item.uvalue = 0;
    item.fvalue = val;
    debugOutput.Append(item);
};
