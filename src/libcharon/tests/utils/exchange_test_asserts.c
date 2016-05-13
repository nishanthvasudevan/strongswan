/*
 * Copyright (C) 2016 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <test_suite.h>

#include "exchange_test_asserts.h"

/*
 * Described in header
 */
bool exchange_test_asserts_hook(listener_t *listener)
{
	listener_hook_assert_t *this = (listener_hook_assert_t*)listener;

	this->count++;
	return TRUE;
}

/*
 * Described in header
 */
bool exchange_test_asserts_ike_updown(listener_t *listener, ike_sa_t *ike_sa,
									  bool up)
{
	listener_hook_assert_t *this = (listener_hook_assert_t*)listener;

	this->count++;
	assert_listener_msg(this->up == up, this, "IKE_SA not '%s'",
						this->up ? "up" : "down");
	return TRUE;
}

/*
 * Described in header
 */
bool exchange_test_asserts_child_updown(listener_t *listener, ike_sa_t *ike_sa,
										child_sa_t *child_sa, bool up)
{
	listener_hook_assert_t *this = (listener_hook_assert_t*)listener;

	this->count++;
	assert_listener_msg(this->up == up, this, "CHILD_SA not '%s'",
						this->up ? "up" : "down");
	return TRUE;
}

/*
 * Described in header
 */
bool exchange_test_asserts_message(listener_t *listener, ike_sa_t *ike_sa,
								message_t *message, bool incoming, bool plain)
{
	listener_message_assert_t *this = (listener_message_assert_t*)listener;

	if (plain && this->incoming == incoming)
	{
		if (this->count >= 0)
		{
			enumerator_t *enumerator;
			int count = 0;
			enumerator = message->create_payload_enumerator(message);
			while (enumerator->enumerate(enumerator, NULL))
			{
				count++;
			}
			enumerator->destroy(enumerator);
			assert_listener_msg(this->count == count, this, "unexpected payload "
								"count in message (%d != %d)", this->count,
								count);
		}
		if (this->payload)
		{
			assert_listener_msg(message->get_payload(message, this->payload),
								this, "expected payload (%N) not found",
								payload_type_names, this->payload);
		}
		if (this->notify)
		{
			assert_listener_msg(message->get_notify(message, this->notify),
								this, "expected notify payload (%N) not found",
								notify_type_names, this->notify);
		}
		return FALSE;
	}
	return TRUE;
}
