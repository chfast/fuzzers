import org.hyperledger.besu.evm.code.CodeV1;
import org.hyperledger.besu.evm.code.EOFLayout.EOFContainerMode;
import org.hyperledger.besu.evm.EVM;
import org.hyperledger.besu.evm.MainnetEVMs;
import org.hyperledger.besu.evm.internal.EvmConfiguration;
import org.apache.tuweni.bytes.Bytes;

public final class BesuFzz {
private final static EVM evm = MainnetEVMs.pragueEOF(EvmConfiguration.DEFAULT);

public static boolean validateEOF(final byte[] rawCode, int length) {
        var code = evm.getCodeUncached(Bytes.wrap(rawCode, 0, length));
        if (!code.isValid())
            return false;
        return !EOFContainerMode.INITCODE.equals(((CodeV1) code).getEofLayout().containerMode().get());
    }
}
