USE [dnstaging]
GO
/****** Object:  Table [dbo].[MobileAuthenticationLogs_20130921]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MobileAuthenticationLogs_20130921](
	[LogDate] [datetime2](0) NOT NULL,
	[AccountID] [int] NOT NULL,
	[AuthenticationLogCode] [tinyint] NOT NULL,
	[AccountAuthenticationKey] [int] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_MobileAuthenticationLogs_20130921_LogDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_MobileAuthenticationLogs_20130921_LogDate] ON [dbo].[MobileAuthenticationLogs_20130921]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_MobileAuthenticationLogs_20130921_AccountID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_MobileAuthenticationLogs_20130921_AccountID] ON [dbo].[MobileAuthenticationLogs_20130921]
(
	[AccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[MobileAuthenticationLogs_20130921]  WITH CHECK ADD  CONSTRAINT [CK_MobileAuthenticationLogs_20130921_LogDate] CHECK  (([LogDate]>='2013-09-21' AND [LogDate]<'2013-09-22'))
GO
ALTER TABLE [dbo].[MobileAuthenticationLogs_20130921] CHECK CONSTRAINT [CK_MobileAuthenticationLogs_20130921_LogDate]
GO
