#include <kablunk.h>
#include <cr.h>


CR_EXPORT int cr_main(struct cr_plugin* ctx, enum cr_op operation)
{
    assert(ctx);
    switch (operation)
    {
		case CR_LOAD: 
			break;
		case CR_UNLOAD: 
			break;
		case CR_CLOSE:	
			break;
		case CR_STEP: 
			break;
    }

	return 0;
}
