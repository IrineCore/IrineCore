USE [dngsm]
GO
/****** Object:  Table [dbo].[AuditLogs]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AuditLogs](
	[AuditLogID] [int] IDENTITY(1,1) NOT NULL,
	[AdminID] [int] NOT NULL,
	[AuditCode] [tinyint] NOT NULL,
	[LogDate] [datetime] NOT NULL,
	[MethodName] [nvarchar](300) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[InputMessage] [nvarchar](max) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[OutputMessage] [nvarchar](max) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
 CONSTRAINT [PK_AuditLogs] PRIMARY KEY CLUSTERED 
(
	[AuditLogID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
