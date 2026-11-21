USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_BanForPublisher]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_BanForPublisher]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_BanForPublisher] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-06-01
description : ?? ?? ???? ?????.
result set : Memo, StartDate, EndDate, RegisterDate
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_BanForPublisher]
	  @nvcAccountName nvarchar(50) --// ?? ?? ??
	, @inyBanCode tinyint --// 1=??, 2=Ban, 3=Unban
	, @nvcMemo nvarchar(300) = NULL --// ?? : NULL / Ban : ?? / Unban : ??
	, @sdtStartDate smalldatetime = NULL --// ?? : NULL / Ban : ?? ?? ?? (?.?.?.?.? ?? ??) / Unban : NULL
	, @sdtEndDate smalldatetime = NULL --// ?? : NULL / Ban : ?? ? ?? (?.?.?.?.? ?? ??) / Unban : NULL
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dtsNow smalldatetime = GETDATE(), @intAccountID int;
DECLARE @tblTemp table (RestraintID int NOT NULL); 

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @intAccountID = AccountID
	FROM dbo.Accounts WITH (NOLOCK)
	WHERE AccountName = @nvcAccountName;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101115;
		GOTO ErrorHandler;
	END

	IF @inyBanCode = 1
		SELECT Memo, StartDate, EndDate, RegisterDate
		FROM dbo.Restraints WITH (NOLOCK)
		WHERE AccountID = @intAccountID AND @dtsNow BETWEEN StartDate AND EndDate AND CancelFlag = CAST(0 AS bit)
		ORDER BY RestraintID;

	ELSE IF @inyBanCode = 2
	BEGIN
		IF EXISTS (
			SELECT *
			FROM dbo.Restraints WITH (NOLOCK)
			WHERE AccountID = @intAccountID AND @dtsNow BETWEEN StartDate AND EndDate AND CancelFlag = CAST(0 AS bit)
		)
			RETURN 0;

		INSERT dbo.Restraints (RestraintTargetCode, AccountID, CharacterID, RestraintReasonID, RestraintTypeCode, AdminID, Memo, RestraintMessage, StartDate, EndDate, RegisterDate, CancelFlag)
		VALUES (1, @intAccountID, NULL, 0, 1, 0, @nvcMemo, NULL, @sdtStartDate, @sdtEndDate, GETDATE(), 0);
	END

	ELSE IF @inyBanCode = 3
	BEGIN
		UPDATE dbo.Restraints
		SET CancelFlag = 1
		OUTPUT inserted.RestraintID
		INTO @tblTemp (RestraintID)
		WHERE AccountID = @intAccountID AND @dtsNow BETWEEN StartDate AND EndDate AND CancelFlag = CAST(0 AS bit);
		INSERT dbo.CanceledRestraints (RestraintID, AdminID, Memo, CancelDate)
		SELECT RestraintID, 0, @nvcMemo, @dtsNow
		FROM @tblTemp;
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
