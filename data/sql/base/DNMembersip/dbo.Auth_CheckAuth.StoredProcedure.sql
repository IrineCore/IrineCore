USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[Auth_CheckAuth]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Auth_CheckAuth]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[Auth_CheckAuth] AS' 
END
GO
/**
version : 10
author : ???
e-mail : b4nfter@eyedentitygames.com
created date : 2010-01-08
description : ???? ?? (??? ?? ??)
return value :
history :
6 = version 4? ?? at 2010-08-12
9 = ?? ?? at 2011-08-01
10 = Try..Catch.. ?? ??     by ??? at 2011-12-01
**/

ALTER PROCEDURE [dbo].[Auth_CheckAuth]
	 @pAccountDBID			INT						--// ?? DBID
	,@pCertifyingKey		BIGINT					--// ???
	,@pCurServerID			INT						--// ???? ?? ????
	,@pCheckAuthLimitSec	INT						--// ?? ?? ???? (??:?)
	,@pWorldID				TINYINT	OUTPUT			--// ?? ID (??)
	,@pSessionID			INT	OUTPUT				--// ?? ID (??)
	,@pCharacterDBID		BIGINT OUTPUT			--// ??? DBID (??)
	,@pAccountName			NVARCHAR(50) OUTPUT		--// ?? ?? (??)
	,@pCharacterName		NVARCHAR(30) OUTPUT		--// ??? ?? (??)
	,@pAccountLevel			TINYINT	OUTPUT			--// ?? ???? (??)
	,@pIsAdult				BIT OUTPUT				--// ?? ?? (??)
	,@pAge					TINYINT OUTPUT			--// ?? (??)
	,@pDailyCreateCount		TINYINT OUTPUT			--// ?? ??? ???? ?? (??)
	,@pPrmInt1				INT OUTPUT				--// ???? 1 (??? ???)
	,@inyLastServerType tinyint = 0 OUTPUT --// ??? ?? ?? ID
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

DECLARE @aErrNo				INT
DECLARE @aRowCnt			INT
DECLARE @aCertifyingKey		BIGINT
DECLARE @aCertifyingStep	TINYINT
DECLARE @aAccessTime		DATETIME

SET @aErrNo = 0;			-- 1 : SQL ????
SET @aCertifyingStep = 0;
BEGIN TRY
	-- ??? ??? ???? ????? ?? ???? ???? ?? ???? ?? ????? ??
	BEGIN TRANSACTION;

	UPDATE dbo.DNAuth
	SET
		 @aCertifyingStep = [CertifyingStep]
		,@aCertifyingKey = [CertifyingKey]
		,@aAccessTime = [AccessTime]
		,@pWorldID = [WorldID]
		,@pSessionID = [SessionID]
		,@pCharacterDBID = [CharacterDBID]
		,@pAccountName = [AccountName]
		,@pCharacterName = [CharacterName]
		,@pAccountLevel = [AccountLevel]
		,@pIsAdult = [IsAdult]
		,@pAge = [Age]
		,@pDailyCreateCount = [DailyCreateCount]
		,@pPrmInt1 = [PrmInt1]
		,@inyLastServerType = LastServerType 
		,CertifyingKey = 0	-- ???? ??? ??
		,CertifyingStep = 2
		,CurServerID = @pCurServerID
		,AccessTime = '1900-01-01'
	WHERE AccountDBID = @pAccountDBID

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1305;		-- ?????? ???? ??
		GOTO ErrorHandler;
	END

	IF (@aCertifyingStep <> 1)
	BEGIN
		SET  @intReturnValue = 1300;	-- ???? ?? ? (????? ??? ??? ?)
		GOTO ErrorHandler;
	END

	IF ((@aCertifyingKey = 0) OR (@aCertifyingKey <> @pCertifyingKey))		-- ?? ???? 0 ? ???? ?? !!!
	BEGIN
		SET  @intReturnValue = 1302;	-- ??? ?? ? (????? ??? ??? ?)
		GOTO ErrorHandler;
	END

	IF (GETDATE() > DATEADD(SECOND, @pCheckAuthLimitSec, @aAccessTime))
	BEGIN
		SET  @intReturnValue = 1304;	-- ?? ?? ????? ??? (????? ??? ??? ?)
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
