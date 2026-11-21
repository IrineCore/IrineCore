USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListRestraint]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListRestraint]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListRestraint] AS' 
END
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-20
description : ?? ??? ?????.
result set : RestraintID, RestraintTargetCode, RestraintReasonID, RestraintTypeCode, AdminID, Memo, RestraintMessage, StartDate, EndDate, RegisterDate, CancelFlag, CancelAdminID, CancelMemo, CancelDate
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101120 = ?? ??? ????.
101131 = @intAccountID ?? @inbCharacterID ? ??
**/

ALTER PROCEDURE [dbo].[P_GetListRestraint]
	  @inyRestraintTargetCode tinyint --// 1=??, 2=???
	, @intAccountID int = NULL --// ?? ??? ??? ??, ??? ?? ID
	, @inbCharacterID bigint = NULL --// ?? ??? ???? ??, ?? ?? ??? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @intAccountID IS NULL AND @inbCharacterID IS NULL
	BEGIN
		SET @intReturnValue = 101131;
		GOTO ErrorHandler;
	END

	/**_# [Restraints], [CanceledRestraints] ???? SELECT ???.*/
	IF @inyRestraintTargetCode = 1
		SELECT R.RestraintID, R.RestraintTargetCode, R.RestraintReasonID, R.RestraintTypeCode, R.AdminID, R.Memo, R.RestraintMessage, R.StartDate, R.EndDate, R.RegisterDate, R.CancelFlag
			, CR.AdminID AS CancelAdminID, CR.Memo AS CancelMemo, CR.CancelDate
		FROM dbo.Restraints R WITH (NOLOCK)
			LEFT OUTER JOIN dbo.CanceledRestraints CR WITH (NOLOCK) ON R.RestraintID = CR.RestraintID
		WHERE R.AccountID = @intAccountID
		ORDER BY R.RestraintID DESC;

	ELSE IF @inyRestraintTargetCode = 2
		SELECT R.RestraintID, R.RestraintTargetCode, R.RestraintReasonID, R.RestraintTypeCode, R.AdminID, R.Memo, R.RestraintMessage, R.StartDate, R.EndDate, R.RegisterDate, R.CancelFlag
			, CR.AdminID AS CancelAdminID, CR.Memo AS CancelMemo, CR.CancelDate
		FROM dbo.Restraints R WITH (NOLOCK)
			LEFT OUTER JOIN dbo.CanceledRestraints CR WITH (NOLOCK) ON R.RestraintID = CR.RestraintID
		WHERE R.CharacterID = @inbCharacterID
		ORDER BY R.RestraintID DESC;

	/**_# ??? ???? ?? ??? 101120? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101120;
		GOTO ErrorHandler;
	END
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
