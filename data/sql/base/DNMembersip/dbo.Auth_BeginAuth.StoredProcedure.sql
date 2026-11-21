USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[Auth_BeginAuth]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Auth_BeginAuth]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[Auth_BeginAuth] AS' 
END
GO
/**
version : 12
author : ???
e-mail : b4nfter@eyedentitygames.com
created date : 2010-01-08
description : Begin Auth
return value :
history :
8 = version 6?? ?? at 2010-08-12
9 = ?? ?? at 2011-07-30
10 = Try..Catch.. ?? ??     by ??? at 2011-12-01
11 = input param. @intVersion ?? by ??? at 2012-03-14
**/

ALTER PROCEDURE [dbo].[Auth_BeginAuth]
	 @pAccountDBID			INT					--// ?? DBID
	,@pCertifyingStep		TINYINT = 0 OUTPUT		--// ?? ?? (??)
	,@pCurServerID			INT					--// ?? ???? ?? ????
	,@pOldServerID			INT = 0 OUTPUT			--// ?? ???? ?? ???? (??)
	,@pWorldID				TINYINT = 0	 OUTPUT		--// ?? ID (??)
	,@pCurSessionID			INT					--// ?? ?? ID
	,@pOldSessionID			INT = 0 OUTPUT			--// ?? ?? ID (??)
	,@pCurCharacterDBID		BIGINT				--// ?? ??? DBID
	,@pOldCharacterDBID		BIGINT = 0 OUTPUT		--// ?? ??? DBID (??)
	,@pAccountName			NVARCHAR(50)		--// ?? ??
	,@pCharacterName		NVARCHAR(30)		--// ??? ??
	,@pAccountLevel			TINYINT				--// ?? ????
	,@pIsAdult				BIT					--// ?? ??
	,@pAge					TINYINT				--// ??
	,@pDailyCreateCount		TINYINT				--// ?? ??? ???? ??
	,@pPrmInt1				INT					--// ???? 1 (??? ???)
	,@intVersion			INT = 10			--// ??? SP? ??
	,@inyChannelPartnerCode tinyint = NULL      --// ??? ??? ??
AS
SET NOCOUNT ON
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	BEGIN TRANSACTION;

	UPDATE dbo.DNAuth
	SET
		@pCertifyingStep = [CertifyingStep]
		,@pWorldID = [WorldID]
		,@pOldServerID = [CurServerID]
		,@pOldSessionID = [SessionID] 
		,@pOldCharacterDBID = [CharacterDBID]
		,CertifyingKey = 0					-- ?? ??? ?? ???? ?? ??? ???? ????
		,CertifyingStep = 2
		,CurServerID = @pCurServerID		-- ???? ?? ???? ??
		,AccessTime = GETDATE()
		,WorldID = 0						-- ??? ??? ?? ID ? ??
		,SessionID = @pCurSessionID
		,CharacterDBID = @pCurCharacterDBID
		,AccountName = @pAccountName
		,CharacterName = @pCharacterName
		,AccountLevel = @pAccountLevel
		,IsAdult = @pIsAdult
		,Age = @pAge
		,DailyCreateCount = @pDailyCreateCount
		,PrmInt1 = @pPrmInt1
		,ChannelPartnerCode = @inyChannelPartnerCode
	WHERE AccountDBID = @pAccountDBID;

	IF @@ROWCOUNT = 0
	BEGIN
		INSERT dbo.DNAuth (AccountDBID, CertifyingKey, CertifyingStep, CurServerID, AccessTime, WorldID, SessionID, CharacterDBID, AccountName, CharacterName, AccountLevel, IsAdult, Age, DailyCreateCount, PrmInt1, ChannelPartnerCode) 
		VALUES (@pAccountDBID, 0, 2, @pCurServerID, GETDATE(), 0, @pCurSessionID, @pCurCharacterDBID, @pAccountName, @pCharacterName, @pAccountLevel, @pIsAdult, @pAge, @pDailyCreateCount, @pPrmInt1, @inyChannelPartnerCode);
	END

	ELSE IF @intVersion = 10 AND (@pCertifyingStep NOT BETWEEN 0 AND 1)
	BEGIN
		SET @intReturnValue = 4;
		GOTO ErrorHandler;
	END

	ELSE IF @intVersion = 11 AND @pCertifyingStep <> 0
	BEGIN
		SET @intReturnValue = 4;
		GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;

END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
