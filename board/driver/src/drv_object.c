#include "drv_config.h"
#include "drv_server.h"
/*
 * define object_info for the number of rt_object_container items.
 */
enum rt_object_info_type
{

    RT_Object_Info_Device,                             /**< The object is a device */
    RT_Object_Info_Unknown,                            /**< The object is unknown. */
};

#define _OBJ_CONTAINER_LIST_INIT(c)     \
    {&(rt_object_container[c].object_list), &(rt_object_container[c].object_list)}
static struct rt_object_information rt_object_container[RT_Object_Info_Unknown] =
{
    /* initialize object container - device */
    {RT_Object_Class_Device, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Device), sizeof(struct rt_device)},
};




/**
 * @ingroup SystemInit
 *
 * This function will initialize system object management.
 *
 * @deprecated since 0.3.0, this function does not need to be invoked
 * in the system initialization.
 */
void rt_system_object_init(void)
{
}

/**
 * @addtogroup KernelObject
 */

/**@{*/

/**
 * This function will return the specified type of object information.
 *
 * @param type the type of object
 * @return the object type information or NULL
 */
struct rt_object_information *
rt_object_get_information(enum rt_object_class_type type)
{
    int index;

    for (index = 0; index < RT_Object_Info_Unknown; index ++)
        if (rt_object_container[index].type == type) return &rt_object_container[index];

    return NULL;
}

/**
 * This function will initialize an object and add it to object system
 * management.
 *
 * @param object the specified object to be initialized.
 * @param type the object type.
 * @param name the object name. In system, the object's name must be unique.
 */
void rt_object_init(struct rt_object         *object,
                    enum rt_object_class_type type,
                    const char               *name)
{
    struct rt_list_node *node = NULL;
    struct rt_object_information *information;

    /* get object information */
    information = rt_object_get_information(type);
    assert_param(information != NULL);

    /* check object type to avoid re-initialization */

    /* enter critical */
    rt_enter_critical();
    /* try to find object */
    for (node  = information->object_list.next;
            node != &(information->object_list);
            node  = node->next)
    {
        struct rt_object *obj;

        obj = rt_list_entry(node, struct rt_object, list);
        if (obj) /* skip warning when disable debug */
        {
            assert_param(obj != object);
        }
    }
    /* leave critical */
    rt_exit_critical();

    /* initialize object's parameters */
    /* set object type to static */
    object->type = type | RT_Object_Class_Static;
    /* copy name */
    strncpy(object->name, name, RT_NAME_MAX);
    /* lock interrupt */
    rt_hw_interrupt_disable();

    {
        /* insert object into information object list */
        rt_list_insert_after(&(information->object_list), &(object->list));
    }

    /* unlock interrupt */
    rt_hw_interrupt_enable();
}

/**
 * This function will detach a static object from object system,
 * and the memory of static object is not freed.
 *
 * @param object the specified object to be detached.
 */
void rt_object_detach(rt_object_t object)
{


    /* object check */
    assert_param(object != NULL);

    /* reset object type */
    object->type = 0;

    /* lock interrupt */
    rt_hw_interrupt_disable();

    /* remove from old list */
    rt_list_remove(&(object->list));

    /* unlock interrupt */
    rt_hw_interrupt_enable();
}

#ifdef RT_USING_HEAP
/**
 * This function will allocate an object from object system
 *
 * @param type the type of object
 * @param name the object name. In system, the object's name must be unique.
 *
 * @return object
 */
rt_object_t rt_object_allocate(enum rt_object_class_type type, const char *name)
{
    struct rt_object *object;
    register rt_base_t temp;
    struct rt_object_information *information;


    RT_DEBUG_NOT_IN_INTERRUPT;

    /* get object information */
    information = rt_object_get_information(type);
    assert_param(information != NULL);

    object = (struct rt_object *)RT_KERNEL_MALLOC(information->object_size);
    if (object == NULL)
    {
        /* no memory can be allocated */
        return NULL;
    }

    /* clean memory data of object */
    rt_memset(object, 0x0, information->object_size);

    /* initialize object's parameters */

    /* set object type */
    object->type = type;

    /* set object flag */
    object->flag = 0;

    /* copy name */
    rt_strncpy(object->name, name, RT_NAME_MAX);

    RT_OBJECT_HOOK_CALL(rt_object_attach_hook, (object));

    /* lock interrupt */
    temp = rt_hw_interrupt_disable();

    {
        /* insert object into information object list */
        rt_list_insert_after(&(information->object_list), &(object->list));
    }

    /* unlock interrupt */
    rt_hw_interrupt_enable(temp);

    /* return object */
    return object;
}

/**
 * This function will delete an object and release object memory.
 *
 * @param object the specified object to be deleted.
 */
void rt_object_delete(rt_object_t object)
{
    register rt_base_t temp;

    /* object check */
    assert_param(object != NULL);
    assert_param(!(object->type & RT_Object_Class_Static));

    RT_OBJECT_HOOK_CALL(rt_object_detach_hook, (object));

    /* reset object type */
    object->type = 0;

    /* lock interrupt */
    temp = rt_hw_interrupt_disable();

    /* remove from old list */
    rt_list_remove(&(object->list));

    /* unlock interrupt */
    rt_hw_interrupt_enable(temp);

    /* free the memory of object */
    RT_KERNEL_FREE(object);
}
#endif

/**
 * This function will judge the object is system object or not.
 * Normally, the system object is a static object and the type
 * of object set to RT_Object_Class_Static.
 *
 * @param object the specified object to be judged.
 *
 * @return RT_TRUE if a system object, RT_FALSE for others.
 */
bool rt_object_is_systemobject(rt_object_t object)
{
    /* object check */
    assert_param(object != NULL);

    if (object->type & RT_Object_Class_Static)
        return true;

    return false;
}

/**
 * This function will return the type of object without
 * RT_Object_Class_Static flag.
 *
 * @param object the specified object to be get type.
 *
 * @return the type of object.
 */
uint8_t rt_object_get_type(rt_object_t object)
{
    /* object check */
    assert_param(object != NULL);

    return object->type & ~RT_Object_Class_Static;
}

/**
 * This function will find specified name object from object
 * container.
 *
 * @param name the specified name of object.
 * @param type the type of object
 *
 * @return the found object or NULL if there is no this object
 * in object container.
 *
 * @note this function shall not be invoked in interrupt status.
 */
rt_object_t rt_object_find(const char *name, uint8_t type)
{
    struct rt_object *object = NULL;
    struct rt_list_node *node = NULL;
    struct rt_object_information *information = NULL;

    /* parameter check */
    if ((name == NULL) || (type > RT_Object_Class_Unknown))
        return NULL;

    /* enter critical */
    rt_enter_critical();

    /* try to find object */
    if (information == NULL)
    {
        information = rt_object_get_information((enum rt_object_class_type)type);
        assert_param(information != NULL);
    }
    for (node  = information->object_list.next;
            node != &(information->object_list);
            node  = node->next)
    {
        object = rt_list_entry(node, struct rt_object, list);
        if (strncmp(object->name, name, RT_NAME_MAX) == 0)
        {
            /* leave critical */
            rt_exit_critical();

            return object;
        }
    }

    /* leave critical */
    rt_exit_critical();

    return NULL;
}

/**@}*/
