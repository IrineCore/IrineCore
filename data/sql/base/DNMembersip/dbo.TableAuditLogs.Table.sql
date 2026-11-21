USE [dnmembership]
GO
/****** Object:  Table [dbo].[TableAuditLogs]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[TableAuditLogs]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[TableAuditLogs](
	[tableAuditLogSN] [bigint] IDENTITY(1,1) NOT NULL,
	[tableName] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[operationCode] [tinyint] NOT NULL,
	[columnNames] [nvarchar](max) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[beforeImages] [nvarchar](max) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[modifier] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[occurredDate] [datetime] NOT NULL,
 CONSTRAINT [PK_TableAuditLogs] PRIMARY KEY CLUSTERED 
(
	[tableAuditLogSN] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1] TEXTIMAGE_ON [DataFG1]
END
GO
