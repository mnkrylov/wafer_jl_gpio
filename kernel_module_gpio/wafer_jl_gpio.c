#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/acpi.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>  // Для работы с cdev

#define ACPI_METHOD_NAME "\\_SB.ISM.SDIO"  // ACPI method name
#define DEVICE_NAME "gpio_wafer"  // Имя устройства
#define CLASS_NAME "gpio_class"   // Имя класса устройства

static dev_t dev_number;           // Мажорный и минорный номер устройства
static struct class *gpio_class;   // Указатель на класс устройства
static struct cdev gpio_cdev;      // Структура символьного устройства

// Function to invoke the SDIO ACPI method
static acpi_status call_acpi_sdio_method(unsigned int arg1)
{
    acpi_status status;
    struct acpi_object_list arg_list;
    union acpi_object args[1];  // Arguments for the SDIO method
    struct acpi_buffer result = { ACPI_ALLOCATE_BUFFER, NULL };  // Buffer for the result

    arg_list.count = 1;
    arg_list.pointer = args;
    args[0].type = ACPI_TYPE_INTEGER;
    args[0].integer.value = arg1;

    // Calling the ACPI method
    status = acpi_evaluate_object(NULL, ACPI_METHOD_NAME, &arg_list, &result);
    if (ACPI_FAILURE(status)) {
        pr_err("Failed to call SDIO ACPI method %s with arg 0x%x\n", ACPI_METHOD_NAME, arg1);
    } 

    // Free the result buffer if the method returned data
    if (result.pointer) {
        kfree(result.pointer);
        result.pointer = NULL;  // Set the pointer to NULL after freeing memory
    }

    return status;
}

// Функция записи в устройство
static ssize_t gpio_wafer_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset)
{
    unsigned int arg1;
    char buf[16];
    
    if (len > sizeof(buf) - 1)
        return -EINVAL;

    // Копируем данные из пользовательского пространства
    if (copy_from_user(buf, buffer, len))
        return -EFAULT;
    
    buf[len] = '\0';  // Null-terminate the string

    // Преобразуем строку в число
    if (kstrtouint(buf, 0, &arg1))
        return -EINVAL;

    // Вызываем метод ACPI с переданным аргументом
    call_acpi_sdio_method(arg1);

    return len;
}

// Операции для символьного устройства
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = gpio_wafer_write,
};

// Init function to create the character device
static int __init gpio_wafer_init(void)
{
    int ret;

    // Шаг 1: Зарегистрировать символьное устройство
    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("Failed to allocate char device region\n");
        return ret;
    }

    // Шаг 2: Инициализация символьного устройства
    cdev_init(&gpio_cdev, &fops);
    gpio_cdev.owner = THIS_MODULE;
    ret = cdev_add(&gpio_cdev, dev_number, 1);
    if (ret < 0) {
        pr_err("Failed to add cdev\n");
        unregister_chrdev_region(dev_number, 1);
        return ret;
    }

    // Шаг 3: Создать класс устройства
    gpio_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(gpio_class)) {
        pr_err("Failed to create class\n");
        cdev_del(&gpio_cdev);
        unregister_chrdev_region(dev_number, 1);
        return PTR_ERR(gpio_class);
    }

    // Шаг 4: Создать файл устройства в /dev
    device_create(gpio_class, NULL, dev_number, NULL, DEVICE_NAME);
    pr_info("/dev/%s device created\n", DEVICE_NAME);

    return 0;
}

// Exit function to clean up
static void __exit gpio_wafer_exit(void)
{
    // Удалить файл устройства
    device_destroy(gpio_class, dev_number);

    // Удалить класс устройства
    class_destroy(gpio_class);

    // Удалить символьное устройство
    cdev_del(&gpio_cdev);

    // Освободить область устройства
    unregister_chrdev_region(dev_number, 1);

    pr_info("/dev/%s device removed\n", DEVICE_NAME);
}

module_init(gpio_wafer_init);
module_exit(gpio_wafer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Krylov MN");
MODULE_DESCRIPTION("ACPI method caller for \\_SB.ISM.SDIO through /dev/gpio_wafer");
