USE [dnserverlog]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[GameLog]
	  @intAccountID int
	, @inbCharacterID bigint
	, @intSessionID int
	, @intWorldID int
	, @intLogType int
	, @insTotalParamCount smallint
	, @vchParam varchar(1024)
AS
SET NOCOUNT ON;

DECLARE @inyLogType tinyint = CAST(ABS(@intLogType) % 256 AS tinyint)
	  , @nvcText nvarchar(2000) = N'[GameLog:' + CAST(@intLogType AS nvarchar(16)) + N' params=' + CAST(@insTotalParamCount AS nvarchar(16)) + N'] ' + CAST(ISNULL(@vchParam, '') AS nvarchar(1800))
	  , @intReturnValue int;

EXEC @intReturnValue = dbo.P_AddCommonLog
	  @inyLogType = @inyLogType
	, @intServerType = 0
	, @intWorldID = @intWorldID
	, @intAccountID = @intAccountID
	, @inbCharacterID = @inbCharacterID
	, @intSessionID = @intSessionID
	, @nvcLogText = @nvcText
	, @intServerID = 0;

RETURN @intReturnValue;
GO
