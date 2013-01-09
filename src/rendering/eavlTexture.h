#ifndef EAVL_TEXTURE_H
#define EAVL_TEXTURE_H

#include "eavlDataSet.h"

// ****************************************************************************
// Class:  eavlTexture
//
// Purpose:
///   Encapsulates an OpenGL/Mesa texture.
//
// Programmer:  Jeremy Meredith
// Creation:    January  9, 2013
//
// Modifications:
// ****************************************************************************
class eavlTexture
{
  protected:
    GLuint id;
    int dim;
  public:
    eavlTexture()
    {
        id = 0;
        dim = 0;
    }
    void Enable()
    {
        if (id == 0)
            return;

        if (dim == 1)
        {
            glBindTexture(GL_TEXTURE_1D, id);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glEnable(GL_TEXTURE_1D);
        }
        else if (dim == 2)
        {
            glBindTexture(GL_TEXTURE_2D, id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glEnable(GL_TEXTURE_2D);
        }
    }
    void Disable()
    {
        if (dim == 1)
            glDisable(GL_TEXTURE_1D);
        else if (dim == 2)
            glDisable(GL_TEXTURE_2D);
    }
    void CreateFromDataSet(eavlDataSet *ds,
                           bool cr, bool cg, bool cb, bool ca)
    {
        eavlRegularStructure reg;
        eavlLogicalStructureRegular *logReg = dynamic_cast<eavlLogicalStructureRegular*>(ds->GetLogicalStructure());
        if (!logReg)
            THROW(eavlException,"Expected regular grid.");

        dim = logReg->GetDimension();
        if (dim != 1 && dim != 2)
            THROW(eavlException, "Expected 1D or 2D regular grid");

        reg = logReg->GetRegularStructure();
        int w = reg.cellDims[0];
        int h = reg.cellDims[1];
        int n = reg.GetNumCells();

        eavlField *field_r = cr ? ds->GetField("r") : NULL;
        eavlField *field_g = cg ? ds->GetField("g") : NULL;
        eavlField *field_b = cb ? ds->GetField("b") : NULL;
        eavlField *field_a = ca ? ds->GetField("a") : NULL;

        if ((cr && field_r->GetAssociation() != eavlField::ASSOC_CELL_SET) ||
            (cg && field_g->GetAssociation() != eavlField::ASSOC_CELL_SET) ||
            (cb && field_b->GetAssociation() != eavlField::ASSOC_CELL_SET) ||
            (ca && field_a->GetAssociation() != eavlField::ASSOC_CELL_SET))
            THROW(eavlException, "Texture component not cell-centered");

        if ((cr && string(field_r->GetArray()->GetBasicType()) != "byte") ||
            (cg && string(field_g->GetArray()->GetBasicType()) != "byte") ||
            (cb && string(field_b->GetArray()->GetBasicType()) != "byte") ||
            (ca && string(field_a->GetArray()->GetBasicType()) != "byte"))
            THROW(eavlException, "Expected byte arrays");

        void *vr = cr ? field_r->GetArray()->GetHostArray() : NULL;
        void *vg = cg ? field_g->GetArray()->GetHostArray() : NULL;
        void *vb = cb ? field_b->GetArray()->GetHostArray() : NULL;
        void *va = ca ? field_a->GetArray()->GetHostArray() : NULL;

#define HW_MIPMAPS

        if (dim == 1)
        {
            glGenTextures(1, &id);
            glBindTexture(GL_TEXTURE_1D, id);
        }
        else if (dim == 2)
        {
            glGenTextures(1, &id);
            glBindTexture(GL_TEXTURE_2D, id);
#ifdef HW_MIPMAPS
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
#endif

            if (ca && !cr && !cg && !cb)
            {
                glTexImage2D(GL_TEXTURE_2D, 0,
                             GL_ALPHA,
                             w, h,
                             0,
                             GL_ALPHA,
                             GL_UNSIGNED_BYTE,
                             va);
            }
            else
            {
                THROW(eavlException, "Unsupported texture type (at the moment)");
            }

            /*
            // here's some rough 2d code for mipmaps, but it's destructive
#ifndef HW_MIPMAPS
            int miplevel = 0;
            int w = fnt->imgw, h = fnt->imgh;
            while (w > 1 || h > 1)
            {
                miplevel++;
                bool do_h = true;
                bool do_v = true;
                if (w > 1) 
                    w /= 2;
                else
                    do_h = false;
                if (h > 1)
                    h /= 2;
                else
                    do_h = false;
                for (int y=0; y<h; y++)
                {
                    for (int x=0; x<w; x++)
                    {
                        if (do_h && do_v)
                        {
                            float a = ((unsigned char*)alpha)[w*2*(y*2 + 0) + (x*2 + 0)];
                            float b = ((unsigned char*)alpha)[w*2*(y*2 + 0) + (x*2 + 1)];
                            float c = ((unsigned char*)alpha)[w*2*(y*2 + 1) + (x*2 + 0)];
                            float d = ((unsigned char*)alpha)[w*2*(y*2 + 1) + (x*2 + 1)];
                            float e = (a+b+c+d)/4.;
                            ((unsigned char*)alpha)[w*y + x] = e;
                        }
                        else if (do_h)
                        {
                            float a = ((unsigned char*)alpha)[w*2*y + (x*2 + 0)];
                            float b = ((unsigned char*)alpha)[w*2*y + (x*2 + 1)];
                            float e = (a+b)/2.;
                            ((unsigned char*)alpha)[w*y + x] = e;
                        }
                        else // (do_v)
                        {
                            float a = ((unsigned char*)alpha)[w*(y*2 + 0) + x];
                            float c = ((unsigned char*)alpha)[w*(y*2 + 1) + x];
                            float e = (a+c)/2.;
                            ((unsigned char*)alpha)[w*y + x] = e;
                        }
                    }
                }
                glTexImage2D(GL_TEXTURE_2D, miplevel,
                             GL_ALPHA,
                             w, h,
                             0,
                             GL_ALPHA,
                             GL_UNSIGNED_BYTE,
                             alpha);
            }
#endif
            */
        }
    }
};

#endif