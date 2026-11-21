USE [dnmembership]
GO
/****** Object:  Table [dbo].[EventRewards]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[EventRewards]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[EventRewards](
	[EventRewardID] [int] IDENTITY(1,1) NOT NULL,
	[ReceiveTypeCode] [tinyint] NOT NULL,
	[TargetTypeCode] [tinyint] NOT NULL,
	[TargetWorldID] [tinyint] NULL,
	[TargetClassCode] [tinyint] NULL,
	[TargetMinLevel] [tinyint] NULL,
	[TargetMaxLevel] [tinyint] NULL,
	[DeleteFlag] [bit] NOT NULL,
	[RegistrationDate] [smalldatetime] NOT NULL,
	[ReserveSendDate] [smalldatetime] NOT NULL,
	[ExpirationDate] [smalldatetime] NOT NULL,
	[EventName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NULL,
	[SenderName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[Content] [nvarchar](200) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[RewardCoin] [bigint] NOT NULL,
	[SystemSendFlag] [bit] NOT NULL,
	[EventRewardTypeCode] [tinyint] NOT NULL,
 CONSTRAINT [PK_EventRewards] PRIMARY KEY CLUSTERED 
(
	[EventRewardID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NN_EventRewards_ExpirationDate_ReserveSendDate_DeleteFlag]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[EventRewards]') AND name = N'IX_NN_EventRewards_ExpirationDate_ReserveSendDate_DeleteFlag')
CREATE NONCLUSTERED INDEX [IX_NN_EventRewards_ExpirationDate_ReserveSendDate_DeleteFlag] ON [dbo].[EventRewards]
(
	[ExpirationDate] ASC,
	[ReserveSendDate] ASC,
	[DeleteFlag] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_EventRewards_DeleteFlag]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[EventRewards] ADD  CONSTRAINT [DF_EventRewards_DeleteFlag]  DEFAULT ((0)) FOR [DeleteFlag]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_EventRewards_SystemSendFlag]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[EventRewards] ADD  CONSTRAINT [DF_EventRewards_SystemSendFlag]  DEFAULT ((0)) FOR [SystemSendFlag]
END
GO
