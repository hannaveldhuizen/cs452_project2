/*
 * Card Driver
 * Author - Hanna Veldhuizen
 * CSC 452
 * Project 2
 */

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/random.h>

#include <linux/uaccess.h>

// array representing the deck of cards
static char cards[52];
// keep track of index in deck so we know where to draw at next
static int index;

/*
 * Gets a single byte. This helper function will be used to shuffle the deck.
 */
unsigned char get_random_byte(int max) {
    unsigned char c;
    get_random_bytes(&c, 1);
    return c%max;
}

/*
 * Shuffles by the deck by swapping cards in the array many times. I chose to loop this over 20 times
 * to hopefully randomize the deck as much as possible. This number can be increased or decreased
 * depending on how much you would like the deck to be shuffled.
 */
static void shuffleDeck(void) {
    int i, j;
    int max = 52;
    for (i = 0; i < 20; i++) {
        char rand = get_random_byte(max);

        for (j = 0; j < max; j++) {
            int swap = (j + rand) % max;

            char temp = cards[j];
            cards[j] = cards[swap];
            cards[swap] = temp;
        }
    }
}

/*
 * card_read is the function called when a process calls read() on
 * /dev/cards.  It writes "Hello, world!" to the buffer passed in the
 * read() call.
 */
static ssize_t card_read(struct file * file, char * buf,
			  size_t count, loff_t *ppos)
{
	char c;
	int i;

    for (i=0; i < count; i++) {
        // shuffle if we are out of cards
        if (index > 51) {
            index = 0;
            shuffleDeck();
        }

        // get card on top of deck
        c = cards[index];
        index++;

        copy_to_user(buf, &c, 1);

        buf++;
        *ppos = *ppos + 1;
    }

    return count;
}

/*
 * The only file operation we care about is read.
 */

static const struct file_operations card_fops = {
	.owner		= THIS_MODULE,
	.read		= card_read,
};

static struct miscdevice card_dev = {
	/*
	 * We don't care what minor number we end up with, so tell the
	 * kernel to just pick one.
	 */
	MISC_DYNAMIC_MINOR,
	/*
	 * Name ourselves /dev/cards.
	 */
	"cards",
	/*
	 * What functions to call when a program performs file
	 * operations on the device.
	 */
	&card_fops
};

static int __init
card_init(void)
{
	int ret;
	int i;

	/*
	 * Create the "card" device in the /sys/class/misc directory.
	 * Udev will automatically create the /dev/cards device using
	 * the default rules.
	 */
	ret = misc_register(&card_dev);
	if (ret)
		printk(KERN_ERR
		       "Unable to register \"Card Driver\" misc device\n");

    // set deck index to 0
    index = 0;

    // initialize cards array
    for (i = 0; i < 52; i++) {
        cards[i] = i + 1;
    }

    // shuffle deck
    shuffleDeck();

	return ret;
}

module_init(card_init);

static void __exit
card_exit(void)
{
	misc_deregister(&card_dev);
}

module_exit(card_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hanna Veldhuizen");
MODULE_DESCRIPTION("Card Driver");
MODULE_VERSION("dev");
