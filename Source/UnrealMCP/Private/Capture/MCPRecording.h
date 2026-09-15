// Copyright (c) 2026 victorvksy. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

class FMCPTcpServer;

/**
 * PIE recording: frames plus a timeline (log, tracked-actor events, frame stats) written to
 * Saved/<StoragePath>/<session>/ while a Play-In-Editor session runs, queried later from disk.
 *
 * Tools: record_pie (async, returns a session id), get_recording_status, stop_recording,
 * get_recording_frames, get_recording_timeline. One session records at a time.
 *
 * Threading (docs/visual-capture/ARCHITECTURE.md, "Phase 2 decisions"): a game-thread ticker
 * enqueues a GPU copy of the PIE viewport's render target into an FRHIGPUTextureReadback and
 * samples actors; a render command polls the readback on later frames and copies the pixels
 * out (Lock/Unlock are render-thread calls) without ever flushing; a per-session worker thread
 * converts, resizes, encodes and writes. The game thread never waits for the GPU or the disk.
 */
namespace MCPRecording
{
	/** Registers the five recording tools with the server and prunes sessions past retention. */
	void RegisterHandlers(FMCPTcpServer& Server);

	/** Finishes any running session (module shutdown). Safe to call when nothing records. */
	void Shutdown();

	/** <Project>/Saved/<StoragePath> (or StoragePath itself when absolute). */
	FString StorageRoot();

	/** Folder of one session; the id is validated by the callers (no path separators). */
	FString SessionDir(const FString& SessionId);

	/** True for ids the tools accept: letters, digits, '-' and '_' only. */
	bool IsValidSessionId(const FString& SessionId);

	/** Removes session folders (those with a manifest.json) older than RetentionDays. */
	int32 PruneOldSessions();

	/** Streams a JSON-lines file; Fn returns false to stop early. An unterminated last line
	 *  (one still being written) is ignored, which is what makes partial sessions queryable. */
	void ForEachJsonLine(const FString& Path, TFunctionRef<bool(const TSharedPtr<FJsonObject>&)> Fn);

	/** Serialises an object to one condensed line (no newline). */
	FString ToJsonLine(const TSharedPtr<FJsonObject>& Obj);
}
