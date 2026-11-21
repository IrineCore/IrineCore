USE [dngsm]
GO
/****** Object:  Table [dbo].[MailServerConfig]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[MailServerConfig](
	[SMTPServerIP] [varchar](15) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[SMTPServerPort] [int] NULL,
	[LoginID] [nvarchar](100) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[LoginPassword] [nvarchar](100) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[SenderAddress] [nvarchar](200) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[UseLogin] [bit] NULL,
	[UseSSL] [bit] NULL,
	[SelectedMethod] [varchar](max) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[ReceiverAddress] [nvarchar](max) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
SET ANSI_PADDING OFF
GO
