USE [dnmembership]
GO
/****** Object:  Table [dbo].[Accounts]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Accounts]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Accounts](
	[AccountID] [int] IDENTITY(1,1) NOT NULL,
	[AccountName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[AccountLevelCode] [tinyint] NOT NULL,
	[GameOption] [binary](20) NULL,
	[CharacterCreateLimit] [tinyint] NOT NULL,
	[LastCharacterCreateDate] [date] NULL,
	[CharacterMaxCount] [tinyint] NOT NULL,
	[LastLoginDate] [datetime2](0) NULL,
	[LastLogoutDate] [datetime2](0) NULL,
	[LastLoginIP] [int] NULL,
	[LastSessionID] [int] NULL,
	[JoinIP] [int] NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[PublisherCode] [tinyint] NOT NULL,
	[GenderCode] [tinyint] NULL,
	[BirthDate] [date] NULL,
	[Passphrase] [binary](20) NULL,
	[KeySettingOption] [binary](80) NULL,
	[SecondAuthPassphrase] [binary](20) SPARSE  NULL,
	[SecondAuthFailCount] [tinyint] NOT NULL,
	[SecondAuthCode] [tinyint] NOT NULL,
	[SecondAuthLockFlag] [bit] NOT NULL,
	[LastSecondAuthNotifyDate] [date] NULL,
	[GamePadOption] [binary](201) NULL,
	[NationalityCode] [tinyint] NULL,
	[SecondAuthResetDate] [smalldatetime] SPARSE  NULL,
	[ChannelPartnerCode] [tinyint] NULL,
	[TotalDT] [int] NULL,
	[AccountKey] [int] NULL,
	[CharacterSortCode] [tinyint] NULL,
	[ConnectNoticeFlag] [bit] NULL,
	[NewbieRewardFlag] [bit] NULL,
	[NewbieRewardDate] [datetime2](0) NULL,
	[ReturnRewardFlag] [bit] NULL,
	[LockFlag] [bit] NULL,
	[NxLoginPwd] [varchar](32) COLLATE Chinese_PRC_CI_AS NULL,
 CONSTRAINT [PK_Accounts] PRIMARY KEY CLUSTERED 
(
	[AccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
SET ANSI_PADDING OFF
GO
SET IDENTITY_INSERT [dbo].[Accounts] ON 

INSERT [dbo].[Accounts] ([AccountID], [AccountName], [AccountLevelCode], [GameOption], [CharacterCreateLimit], [LastCharacterCreateDate], [CharacterMaxCount], [LastLoginDate], [LastLogoutDate], [LastLoginIP], [LastSessionID], [JoinIP], [RegisterDate], [PublisherCode], [GenderCode], [BirthDate], [Passphrase], [KeySettingOption], [SecondAuthPassphrase], [SecondAuthFailCount], [SecondAuthCode], [SecondAuthLockFlag], [LastSecondAuthNotifyDate], [GamePadOption], [NationalityCode], [SecondAuthResetDate], [ChannelPartnerCode], [TotalDT], [AccountKey], [CharacterSortCode], [ConnectNoticeFlag], [NewbieRewardFlag], [NewbieRewardDate], [ReturnRewardFlag], [LockFlag], [NxLoginPwd]) VALUES (21, N'mengmenggu', 0, NULL, 7, NULL, 4, NULL, NULL, NULL, NULL, NULL, CAST(N'2014-10-06T01:04:37.0000000' AS DateTime2), 4, NULL, NULL, NULL, NULL, NULL, 0, 1, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, N'799852BB2314C25D472E60762AE15969')
SET IDENTITY_INSERT [dbo].[Accounts] OFF
GO
/****** Object:  Index [IX_NN_Accounts_AccountLevelCode]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Accounts]') AND name = N'IX_NN_Accounts_AccountLevelCode')
CREATE NONCLUSTERED INDEX [IX_NN_Accounts_AccountLevelCode] ON [dbo].[Accounts]
(
	[AccountLevelCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_Accounts_RegisterDate]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Accounts]') AND name = N'IX_NN_Accounts_RegisterDate')
CREATE NONCLUSTERED INDEX [IX_NN_Accounts_RegisterDate] ON [dbo].[Accounts]
(
	[RegisterDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_UN_Accounts_AccountName]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Accounts]') AND name = N'IX_UN_Accounts_AccountName')
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_Accounts_AccountName] ON [dbo].[Accounts]
(
	[AccountName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_Accounts_SecondAuthFailCount]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[Accounts] ADD  CONSTRAINT [DF_Accounts_SecondAuthFailCount]  DEFAULT ((0)) FOR [SecondAuthFailCount]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_Accounts_SecondAuthCode]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[Accounts] ADD  CONSTRAINT [DF_Accounts_SecondAuthCode]  DEFAULT ((1)) FOR [SecondAuthCode]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_Accounts_SecondAuthLockFlag]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[Accounts] ADD  CONSTRAINT [DF_Accounts_SecondAuthLockFlag]  DEFAULT ((0)) FOR [SecondAuthLockFlag]
END
GO
