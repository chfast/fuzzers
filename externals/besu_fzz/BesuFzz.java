import org.hyperledger.besu.evm.Code;
import org.hyperledger.besu.evm.EVM;
import org.hyperledger.besu.evm.MainnetEVMs;
import org.hyperledger.besu.evm.internal.EvmConfiguration;
import org.apache.tuweni.bytes.Bytes;

public final class BesuFzz {
private final static EVM evm = MainnetEVMs.pragueEOF(EvmConfiguration.DEFAULT);

public static int validateEOF(final byte[] rawCode) {
        Code code = evm.getCodeUncached(Bytes.wrap(rawCode));
        return code.isValid() ? 1 : 0;
    }
}
