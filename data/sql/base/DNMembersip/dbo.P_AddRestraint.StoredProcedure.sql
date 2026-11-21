USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddRestraint]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddRestraint]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddRestraint] AS' 
END
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-07
description : ?? ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ???? ????.
101117 = ???? ???? ????.
101118 = ?? ??? ??????.
101129 = ?? ??? ?? ??? ??? ?? ??????.
101130 = ?? ???? ?? ??? ??? ?? ??????.
101131 = @intAccountID ?? @inbCharacterID ? ??
**/

ALTER PROCEDURE [dbo].[P_AddRestraint]
	  @inyRestraintTargetCode tinyint --// 1=??, 2=???
	, @intAccountID int = NULL --// ?? ??? ??? ??, ??? ?? ID
	, @inbCharacterID bigint = NULL --// ?? ??? ???? ??, ?? ?? ??? ID
	, @intRestraintReasonID int --// ?? ?? ID (DNGSM.dbo.RestraintReasons.RestraintReasonID? ?????.)
	, @inyRestraintTypeCode tinyint --// 1=?? ??, 2=?? ??, 3=?? ??
	, @intAdminID int --// ??? ??? ID (????? ?? ???? ?? 0? ?????.)
	, @nvcMemo nvarchar(300) = NULL --// (optional) ?? ??
	, @nvcRestraintMessage nvarchar(300) --// (optional) ????? ??? ?? ???
	, @sdtStartDate smalldatetime --// ?? ?? ?? (?.?.?.?.? ?? ??)
	, @sdtEndDate smalldatetime --// ?? ? ?? (?.?.?.?.? ?? ??)
	, @intRestraintID int = NULL OUTPUT --// ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dtsNow smalldatetime = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intAccountID = CASE @inyRestraintTargetCode WHEN 1 THEN @intAccountID ELSE NULL END;
	SET @inbCharacterID = CASE @inyRestraintTargetCode WHEN 2 THEN @inbCharacterID ELSE NULL END;

	IF @intAccountID IS NULL AND @inbCharacterID IS NULL
	BEGIN
		SET @intReturnValue = 101131;
		GOTO ErrorHandler;
	END

	/**_# ?? ??? ?? ??? ??? ???? ??? 101129? ?????.*/
	IF @inyRestraintTargetCode = 1 AND EXISTS (
		SELECT *
		FROM dbo.Restraints WITH (NOLOCK)
		WHERE RestraintTargetCode = 1 AND AccountID = @intAccountID
			AND RestraintTypeCode = @inyRestraintTypeCode AND @dtsNow BETWEEN StartDate AND EndDate AND CancelFlag = CAST(0 AS bit)
	)
	BEGIN
		SET @intReturnValue = 101129;
		GOTO ErrorHandler;
	END

	IF @inyRestraintTargetCode = 2
	BEGIN
		/**_# ??? ?? & ??? ????? 101118? ?????.*/
		IF EXISTS (SELECT * FROM dbo.DeletedCharacters WITH (NOLOCK) WHERE CharacterID = @inbCharacterID)
		BEGIN
			SET @intReturnValue = 101118;
			GOTO ErrorHandler;
		END

		/**_# ?? ???? ?? ??? ??? ???? ??? 101130? ?????.*/
		IF EXISTS (
			SELECT *
			FROM dbo.Restraints WITH (NOLOCK)
			WHERE RestraintTargetCode = 2 AND CharacterID = @inbCharacterID
				AND RestraintTypeCode = @inyRestraintTypeCode AND @dtsNow BETWEEN StartDate AND EndDate AND CancelFlag = CAST(0 AS bit)
		)
		BEGIN
			SET @intReturnValue = 101130;
			GOTO ErrorHandler;
		END
	END

	/**_# [Restraints] ???? INSERT???.*/
	/**_## ????? ??? ???? ??? 101115? ?????.*/
	/**_## ????? ???? ???? ??? 101117? ?????.*/
	INSERT dbo.Restraints (RestraintTargetCode, AccountID, CharacterID, RestraintReasonID, RestraintTypeCode, AdminID, Memo, RestraintMessage, StartDate, EndDate, RegisterDate, CancelFlag)
	VALUES (@inyRestraintTargetCode, @intAccountID, @inbCharacterID, @intRestraintReasonID, @inyRestraintTypeCode, @intAdminID, @nvcMemo, @nvcRestraintMessage, @sdtStartDate, @sdtEndDate, GETDATE(), 0);

	SET @intRestraintID = SCOPE_IDENTITY();
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Accounts_Restraints_AccountID%'
		SET @intReturnValue = 101115;

	ELSE IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Characters_Restraints_CharacterID%'
		SET @intReturnValue = 101117;

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
