#include <runtime.h>
#include <unistd.h>

struct bag {
    table listeners;
    table eav;
    table ave;
    value uuid;
    heap h;
};

table level_fetch(heap h, table current, value key) {
    table next_level = table_find(current, key);
    if(!next_level) {
        next_level = create_value_table(h);
        table_set(current, key, next_level);
    }
    return next_level;
}

void full_scan(bag b, three_listener f)
{
    table_foreach(b->eav, e, al) {
        table_foreach(((table)al), a, vl) {
            table_foreach(((table)vl), v, _) {
                apply(f, e, a, v, etrue);
            }
        }
    }
}

void eav_scan(bag b, value e, value a, value v, zero_listener f)
{
    table al = table_find(b->eav, e);
    if(al) {
        table vl = table_find(al, a);
        if(vl) {
            if (table_elements(vl) > 0) {
                apply(f, etrue);
            }
        }
    }
}

void ea_scan(bag b, value e, value a, one_listener f)
{
    table al = table_find(b->eav, e);
    if(al) {
        table vl = table_find(al, a);
        if(vl) {
            table_foreach(vl, v, _) {
                apply(f, v, etrue);
            }
        }
    }
}

void e_scan(bag b, value e, two_listener f)
{
    table al = table_find(b->eav, e);
    if(al) {
        table_foreach(al, a, vl) {
            table_foreach(((table)vl), v, _) {
                apply(f, a, v, etrue);
            }
        }
    }
}

void av_scan(bag b, value a, value v, one_listener f)
{
    table al = table_find(b->ave, a);
    if(al) {
        table vl = table_find(al, v);
        if(vl) {
            table_foreach(vl, e, _) {
                apply(f, e, etrue);
            }
        }
    }
}

// its probably going to be better to keep a global guy with
// reference counts because of the object sharing, but this is
// ok for today
bag create_bag(value bag_id)
{
    heap h = allocate_rolling(pages);
    bag b = allocate(h, sizeof(struct bag));
    b->h = h ;
    b->eav = create_value_table(h);
    b->ave = create_value_table(h);

    return b;
}

void edb_insert(bag b, value e, value a, value v)
{
    // EAV
    {
        table el = level_fetch(b->h, b->eav, e);
        table al = level_fetch(b->h, el, a);
        table tail = level_fetch(b->h, al, v);
    }

    // AVE
    {
        table al = level_fetch(b->h, b->ave, a);
        table vl = level_fetch(b->h, al, v);
        table tail = level_fetch(b->h, vl, e);
    }
}


static void indent(buffer out, int x)
{
    for (int i= 0; i< x; i++)
        buffer_write_byte(out, ' ');
}

static void value_print(buffer out, value v)
{
    switch(type_of(v)) {
    case uuid_space:
        bprintf(out , "%X", wrap_buffer(init, v, UUID_LENGTH));
        break;
        //    case float_space:
        //        break;
    case estring_space:
        {
            string_intermediate si = v;
            bprintf(out , "\"");
            buffer_append(out, si->body, si->length);
            bprintf(out , "\"");
        }
        break;
    default:
        write (1, "wth!@\n", 6);
    }

}

string bag_dump(heap h, bag b)
{
    buffer out = allocate_string(h);
    table_foreach(b->eav, e, avl) {
        int start = buffer_length(out);
        int afirst = 0;
        value_print(out, e);
        indent(out, 1);
        int ind = buffer_length(out)-start;

        table_foreach(((table)avl), a, vl) {
            int start = buffer_length(out);
            int vfirst = 0;
            if (afirst++) indent(out, ind);
            value_print(out, a);
            indent(out, 1);
            int ind2 = ind+buffer_length(out)-start;
            table_foreach(((table)vl), v, _) {
                if (vfirst++) indent(out, ind2);
                value_print(out, v);
                buffer_write_byte(out, '\n');
            }
        }
    }
    return out;
}

void edb_remove(bag b, value e, value a, value v)
{
    error("but we went to so much trouble!\n");
}


