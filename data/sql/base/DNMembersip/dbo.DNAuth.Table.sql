USE [dnmembership]
GO
/****** Object:  Table [dbo].[DNAuth]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DNAuth]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[DNAuth](
	[AccountDBID] [int] NOT NULL,
	[CertifyingKey] [bigint] NOT NULL,
	[CertifyingStep] [tinyint] NOT NULL,
	[CurServerID] [int] NOT NULL,
	[AccessTime] [datetime] NOT NULL,
	[WorldID] [tinyint] NOT NULL,
	[SessionID] [int] NOT NULL,
	[CharacterDBID] [bigint] NOT NULL,
	[AccountName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[CharacterName] [nvarchar](30) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[AccountLevel] [tinyint] NOT NULL,
	[IsAdult] [bit] NOT NULL,
	[Age] [tinyint] NOT NULL,
	[DailyCreateCount] [tinyint] NOT NULL,
	[PrmInt1] [int] NOT NULL,
	[LastServerType] [tinyint] NOT NULL,
	[NationalityCode] [tinyint] NULL,
	[ChannelPartnerCode] [tinyint] NULL,
 CONSTRAINT [PK_DNAuth] PRIMARY KEY CLUSTERED 
(
	[AccountDBID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NN_DNAuth_CurServerID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[DNAuth]') AND name = N'IX_NN_DNAuth_CurServerID')
CREATE NONCLUSTERED INDEX [IX_NN_DNAuth_CurServerID] ON [dbo].[DNAuth]
(
	[CurServerID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_DNAuth_WorldID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[DNAuth]') AND name = N'IX_NN_DNAuth_WorldID')
CREATE NONCLUSTERED INDEX [IX_NN_DNAuth_WorldID] ON [dbo].[DNAuth]
(
	[WorldID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNAuth_CertifyingKey]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNAuth] ADD  CONSTRAINT [DF_DNAuth_CertifyingKey]  DEFAULT ((0)) FOR [CertifyingKey]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNAuth_CertifyingStep]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNAuth] ADD  CONSTRAINT [DF_DNAuth_CertifyingStep]  DEFAULT ((0)) FOR [CertifyingStep]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNAuth_CurServerID]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNAuth] ADD  CONSTRAINT [DF_DNAuth_CurServerID]  DEFAULT ((0)) FOR [CurServerID]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNAuth_AccessTime]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNAuth] ADD  CONSTRAINT [DF_DNAuth_AccessTime]  DEFAULT ('1900-01-01') FOR [AccessTime]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNAuth_WorldID]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNAuth] ADD  CONSTRAINT [DF_DNAuth_WorldID]  DEFAULT ((0)) FOR [WorldID]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNAuth_SessionID]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNAuth] ADD  CONSTRAINT [DF_DNAuth_SessionID]  DEFAULT ((0)) FOR [SessionID]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNAuth_CharacterDBID]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNAuth] ADD  CONSTRAINT [DF_DNAuth_CharacterDBID]  DEFAULT ((0)) FOR [CharacterDBID]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNAuth_AccountName]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNAuth] ADD  CONSTRAINT [DF_DNAuth_AccountName]  DEFAULT ('') FOR [AccountName]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNAuth_CharacterName]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNAuth] ADD  CONSTRAINT [DF_DNAuth_CharacterName]  DEFAULT ('') FOR [CharacterName]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNAuth_AccountLevel]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNAuth] ADD  CONSTRAINT [DF_DNAuth_AccountLevel]  DEFAULT ((0)) FOR [AccountLevel]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNAuth_IsAdult]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNAuth] ADD  CONSTRAINT [DF_DNAuth_IsAdult]  DEFAULT ((0)) FOR [IsAdult]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNAuth_Age]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNAuth] ADD  CONSTRAINT [DF_DNAuth_Age]  DEFAULT ((0)) FOR [Age]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNAuth_DailyCreateCount]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNAuth] ADD  CONSTRAINT [DF_DNAuth_DailyCreateCount]  DEFAULT ((0)) FOR [DailyCreateCount]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNAuth_PrmInt1]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNAuth] ADD  CONSTRAINT [DF_DNAuth_PrmInt1]  DEFAULT ((0)) FOR [PrmInt1]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DNAuth_LastServerType]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DNAuth] ADD  CONSTRAINT [DF_DNAuth_LastServerType]  DEFAULT ((0)) FOR [LastServerType]
END
GO
IF NOT EXISTS (SELECT * FROM sys.check_constraints WHERE object_id = OBJECT_ID(N'[dbo].[CK_DNAuth_CertifyingStep]') AND parent_object_id = OBJECT_ID(N'[dbo].[DNAuth]'))
ALTER TABLE [dbo].[DNAuth]  WITH CHECK ADD  CONSTRAINT [CK_DNAuth_CertifyingStep] CHECK  (([CertifyingStep]>=(0) AND [CertifyingStep]<=(2)))
GO
IF  EXISTS (SELECT * FROM sys.check_constraints WHERE object_id = OBJECT_ID(N'[dbo].[CK_DNAuth_CertifyingStep]') AND parent_object_id = OBJECT_ID(N'[dbo].[DNAuth]'))
ALTER TABLE [dbo].[DNAuth] CHECK CONSTRAINT [CK_DNAuth_CertifyingStep]
GO
