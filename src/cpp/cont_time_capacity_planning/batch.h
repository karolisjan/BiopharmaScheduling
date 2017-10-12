#ifndef __BATCH_H__
#define __BATCH_H__

namespace types
{
	struct Batch
	{
		Batch() : 
			product(0),
			stored_at(0),
			expires_at(0)
		{}

		int product;
		double stored_at, expires_at;
	};
}

#endif // !__BATCH_H__
