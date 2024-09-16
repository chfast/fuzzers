import org.hyperledger.besu.evm.code.CodeV1;
import org.hyperledger.besu.evm.code.EOFLayout.EOFContainerMode;
import org.hyperledger.besu.evm.EVM;
import org.hyperledger.besu.evm.MainnetEVMs;
import org.hyperledger.besu.evm.internal.EvmConfiguration;
import org.apache.tuweni.bytes.Bytes;

public final class BesuFzz {
private final static EVM evm = MainnetEVMs.pragueEOF(EvmConfiguration.DEFAULT);

public static int validateEOF(final byte[] rawCode, int length) {
        var code = evm.getCodeUncached(Bytes.wrap(rawCode, 0, length));
        if (!code.isValid())
            return 0;

        var mode = ((CodeV1) code).getEofLayout().containerMode().get();
        var rt = !EOFContainerMode.INITCODE.equals(mode) ? 1 : 0;
        var ic = !EOFContainerMode.RUNTIME.equals(mode) ? 2 : 0;
        return ic | rt;
    }
}
