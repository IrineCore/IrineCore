USE [dnmembership]
GO
/****** Object:  Table [dbo].[GiftSendSchedule]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[GiftSendSchedule]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[GiftSendSchedule](
	[GiftSendScheduleID] [int] IDENTITY(1,1) NOT NULL,
	[GiftExpireDate] [datetime] NOT NULL,
	[GiftSendDate] [datetime] NOT NULL,
	[GiftTypeCode] [tinyint] NOT NULL,
	[GiftSenderName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[GiftLifespan] [int] NOT NULL,
	[IP] [varchar](15) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[ProductID] [int] NOT NULL,
	[SelectedItemIDs] [varchar](max) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[GiftMemo] [nvarchar](100) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[PayMethodCode] [tinyint] NOT NULL,
	[NewFlag] [bit] NULL,
	[DeleteFlag] [bit] NOT NULL,
	[ItemOptions] [varchar](max) COLLATE Chinese_PRC_CI_AS NULL,
 CONSTRAINT [PK_GiftSendSchedule] PRIMARY KEY CLUSTERED 
(
	[GiftSendScheduleID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1] TEXTIMAGE_ON [DataFG1]
END
GO
SET ANSI_PADDING OFF
GO
/****** Object:  Index [IX_NN_GiftSendSchedule_GiftTypeCode_GiftSendDate_GiftExpireDate_DeleteFlag]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[GiftSendSchedule]') AND name = N'IX_NN_GiftSendSchedule_GiftTypeCode_GiftSendDate_GiftExpireDate_DeleteFlag')
CREATE NONCLUSTERED INDEX [IX_NN_GiftSendSchedule_GiftTypeCode_GiftSendDate_GiftExpireDate_DeleteFlag] ON [dbo].[GiftSendSchedule]
(
	[GiftTypeCode] ASC,
	[GiftSendDate] ASC,
	[GiftExpireDate] ASC,
	[DeleteFlag] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_GiftSendSchedule_DeleteFlag]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[GiftSendSchedule] ADD  CONSTRAINT [DF_GiftSendSchedule_DeleteFlag]  DEFAULT ((0)) FOR [DeleteFlag]
END
GO
