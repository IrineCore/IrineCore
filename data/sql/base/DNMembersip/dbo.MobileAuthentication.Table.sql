USE [dnmembership]
GO
/****** Object:  Table [dbo].[MobileAuthentication]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[MobileAuthentication]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[MobileAuthentication](
	[AccountID] [int] NOT NULL,
	[AccountAuthenticationKey] [int] NULL,
	[AccountAuthenticationKeyIssueDate] [datetime] NULL,
	[AccessKey] [varchar](max) COLLATE Chinese_PRC_CI_AS NULL,
	[EncryptedPassPhase] [binary](20) NULL,
	[EncryptedSerial] [binary](20) NULL,
 CONSTRAINT [PK_MobileAuthentication_AccountID] PRIMARY KEY CLUSTERED 
(
	[AccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1] TEXTIMAGE_ON [DataFG1]
END
GO
SET ANSI_PADDING OFF
GO
/****** Object:  Index [IX_NN_MobileAuthentication_AccountAuthenticationKey]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[MobileAuthentication]') AND name = N'IX_NN_MobileAuthentication_AccountAuthenticationKey')
CREATE NONCLUSTERED INDEX [IX_NN_MobileAuthentication_AccountAuthenticationKey] ON [dbo].[MobileAuthentication]
(
	[AccountAuthenticationKey] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_MobileAuthentication_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[MobileAuthentication]'))
ALTER TABLE [dbo].[MobileAuthentication]  WITH CHECK ADD  CONSTRAINT [FK_Accounts_MobileAuthentication_AccountID] FOREIGN KEY([AccountID])
REFERENCES [dbo].[Accounts] ([AccountID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_MobileAuthentication_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[MobileAuthentication]'))
ALTER TABLE [dbo].[MobileAuthentication] CHECK CONSTRAINT [FK_Accounts_MobileAuthentication_AccountID]
GO
