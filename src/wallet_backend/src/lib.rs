// The number of iterations of PBKDF2 to perform on the wallet
// password.
pub const PBKDF2_ITERATIONS: u64 = 500000;

// What version of the file format are we on (to make it easier to
// upgrade the wallet format in the future).
pub const WALLET_FILE_FORMAT_VERSION: u16 = 0;

// How large should the m_lastKnownBlockHashes container be.
pub const LAST_KNOWN_BLOCK_HASHES_SIZE: u32 = 100;

// Save a block hash checkpoint every BLOCK_HASH_CHECKPOINTS_INTERVAL
// blocks.
pub const BLOCK_HASH_CHECKPOINTS_INTERVAL: u32 = 5000;

// When we get the global indexes, we pass in a range of blocks, to obscure
// which transactions we are interested in - the ones that belong to us.
// To do this, we get the global indexes for all transactions in a range.
//
// For example, if we want the global indexes for a transaction in block
// 17, we get all the indexes from block 10 to block 20.
//
// This value determines how many blocks to take from.
pub const GLOBAL_INDEXES_OBSCURITY: u64 = 10;

// The maximum amount of blocks we can have waiting to be processed in
// the queue. If we exceed this, we will wait till it drops below this
// amount.
pub const MAXIMUM_SYNC_QUEUE_SIZE: u32 = 1000;

pub mod wallet_backend;
