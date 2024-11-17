
inline bool editbuf::regionIsActive() const
{
    return flags & EB_MARKSET;
}

inline void editbuf::regionMarkActive()
{
    flags |= EB_MARKSET;
}

inline void editbuf::regionMarkInactive()
{
    flags &= ~EB_MARKSET;
}

inline bool editbuf::isDirty()
{
    return flags & EB_DIRTY;
}

inline void editbuf::markDirty()
{
    flags |= EB_DIRTY;
}
