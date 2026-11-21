USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_GetListEventReward]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DLS_GetListEventReward]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DLS_GetListEventReward] AS' 
END
GO
/**
version : 4
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-08-24

description :
	??? ?? ?? ??
result set : 
	EventRewardID / int / ??? ?? ID
	ReceiveTypeCode / tinyint / ?? ?? (1=????, 2=????)
	TargetTypeCode / tinyint / ?? ?? (1=????, 2=????, 3=?????, 4=?????, 5=????)
	TargetWorldID / tinyint / ?? ID
	TargetClassCode / tinyint / ???? (1=???, 2=??, 3=????, 4=???, 5=????, 6=??)
	TargetMinLevel / tinyint / ?? ??
	TargetMaxLevel / tinyint / ?? ??
	DeleteFlag / bit / ?? ?? (0=??, 1=??)
	RegistrationDate / smalldatetime / ?? ??
	ReserveSendDate / smalldatetime / ?? ?? ??
	ExpirationDate / smalldatetime / ?? ??
	EventName / nvarchar(50) / ??? ??
	SenderName / nvarchar(50) / ??? ?? ??
	Content / nvarchar(200) / ??
	RewardCoin / bigint / ?? ????
	SystemSendFlag / bit / ??? ?? ?? (0=?????, 1=?????)
	EventRewardTypeCode / tinyint / 1=??? ??, 2=??? ??? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_DLS_GetListEventReward]
	  @intEventRewardID int = NULL --// ??? ?? ID
	, @sdtRegistrationStartDate smalldatetime = NULL --// ?? ?? (??)
	, @sdtRegistrationEndDate smalldatetime = NULL --// ?? ?? (?)
	, @sdtReserveSendStartDate smalldatetime = NULL --// ?? ?? ?? (??)
	, @sdtReserveSendEndDate smalldatetime = NULL --// ?? ?? ?? (?)
	, @sdtExpirationStartDate smalldatetime = NULL --// ?? ?? (??)
	, @sdtExpirationEndDate smalldatetime = NULL --// ?? ?? (?)
	, @nvcEventName nvarchar(50) = NULL --// ??? ?? (LIKE ??)
	, @nvcSenderName nvarchar(50) = NULL --// ??? ?? ?? (= ??)
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

	/**_# ??? ?? ?? ?? */
	IF @intEventRewardID IS NOT NULL
	BEGIN
		SELECT EventRewardID, ReceiveTypeCode, TargetTypeCode, TargetWorldID, TargetClassCode, TargetMinLevel, TargetMaxLevel, DeleteFlag, RegistrationDate, ReserveSendDate, ExpirationDate, EventName, SenderName, Content, RewardCoin, SystemSendFlag, EventRewardTypeCode
		  FROM dbo.EventRewards WITH (NOLOCK)
		 WHERE EventRewardID = @intEventRewardID;
	END
	ELSE 
	BEGIN
		SELECT EventRewardID, ReceiveTypeCode, TargetTypeCode, TargetWorldID, TargetClassCode, TargetMinLevel, TargetMaxLevel, DeleteFlag, RegistrationDate, ReserveSendDate, ExpirationDate, EventName, SenderName, Content, RewardCoin, SystemSendFlag, EventRewardTypeCode
		  FROM dbo.EventRewards WITH (NOLOCK)
		 WHERE (@sdtRegistrationStartDate IS NULL OR RegistrationDate >= @sdtRegistrationStartDate)
		   AND (@sdtRegistrationEndDate IS NULL OR RegistrationDate <= @sdtRegistrationEndDate)
		   AND (@sdtReserveSendStartDate IS NULL OR ReserveSendDate >= @sdtReserveSendStartDate)
		   AND (@sdtReserveSendEndDate IS NULL OR ReserveSendDate <= @sdtReserveSendEndDate)
		   AND (@sdtExpirationStartDate IS NULL OR ExpirationDate >= @sdtExpirationStartDate)
		   AND (@sdtExpirationEndDate IS NULL OR ExpirationDate <= @sdtExpirationEndDate)
		   AND (@nvcEventName IS NULL OR EventName LIKE N'%' + @nvcEventName + N'%')
		   AND (@nvcSenderName IS NULL OR SenderName = @nvcSenderName);
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
